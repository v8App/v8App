// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <algorithm>

#include "uuid/uuid.h"

#include "Logging/LogMacros.h"
#include "Utils/Format.h"

#include "CppBridge/CallbackRegistry.h"
#include "JSApp.h"
#include "V8SnapshotProvider.h"
#include "JSContext.h"
#include "JSUtilities.h"
#include "JSContextSnapData.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContext::JSContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
                             std::filesystem::path inEntryPoint, size_t inContextIndex,
                             bool inSupportsSnapshot, SnapshotMethod inSnapMethod)
            : m_Runtime(inRuntime), m_Name(inName), m_SnapIndex(inContextIndex), m_Namespace(inNamespace),
              m_EntryPoint(inEntryPoint), m_SupportsSnapshots(inSupportsSnapshot), m_SnapMethod(inSnapMethod)
        {
            CHECK_NOT_NULL(m_Runtime.get());
            // We use the : a seperator for shadow names
            m_Name.erase(std::remove(m_Name.begin(), m_Name.end(), ':'), m_Name.end());
        }

        JSContext::~JSContext()
        {
            // make sure the context is disposed of
            DisposeContext();
        }

        JSContext::JSContext(JSContext &&inContext)
        {
            MoveContext(std::move(inContext));
        }

        JSContext &JSContext::operator=(JSContext &&inContext)
        {
            MoveContext(std::move(inContext));
            return *this;
        }

        V8Isolate *JSContext::GetIsolate()
        {
            return (m_Runtime == nullptr) ? nullptr : m_Runtime->GetIsolate();
        }

        JSAppSharedPtr JSContext::GetJSApp()
        {
            return (m_Runtime == nullptr) ? nullptr : m_Runtime->GetApp();
        }

        JSContextSharedPtr JSContext::GetJSContextFromV8Context(V8LContext inContext)
        {
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(inContext->GetAlignedPointerFromEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return JSContextSharedPtr();
            }
            return weakPtr->lock();
        }

        V8LContext JSContext::GetLocalContext()
        {
            V8Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return V8LContext();
            }
            return m_Context.Get(isolate);
        }

        void JSContext::SetupShadowRealmCallback(V8Isolate *inIsolate)
        {
            inIsolate->SetHostCreateShadowRealmContextCallback(JSContext::HostCreateShadowRealmContext);
        }

        void JSContext::DisposeV8Context(bool forSnapshot)
        {
            V8Isolate *isolate = GetIsolate();
            if (isolate != nullptr)
            {
                V8Isolate::Scope iScope(isolate);
                V8HandleScope hScope(isolate);
                if (m_Context.IsEmpty() == false)
                {
                    V8LContext context = m_Context.Get(isolate);

                    // delink the class pointer
                    JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(0));
                    if (weakPtr != nullptr)
                    {
                        delete weakPtr;
                    }
                    // if we are doing a snapshot then we'll be serializeing the context so we set the pointer
                    // for the JSContext directly
                    if (forSnapshot)
                    {
                        context->SetAlignedPointerInEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr), this);
                    }
                    else
                    {
                        context->SetAlignedPointerInEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr), nullptr);
                    }
                }
                m_Context.Reset();
            }
        }

        std::string JSContext::GenerateShadowName()
        {
            constexpr const char *shadow_str = "shadow";
            constexpr const char *shadow_delim = ":";

            std::size_t delim_pos = m_Name.find_first_of(shadow_delim);
            std::size_t shadow_count = 0;
            std::string name = m_Name.substr(0, delim_pos);

            if (delim_pos != m_Name.npos)
            {
                name = m_Name.substr(0, delim_pos);
                delim_pos = m_Name.find_first_of(shadow_delim, delim_pos + 1);
                if (delim_pos != m_Name.npos)
                {
                    std::string intStrVal = m_Name.substr(delim_pos + 1);
                    shadow_count = std::atoi(intStrVal.c_str());
                }
            }
            shadow_count++;
            return name + shadow_delim + shadow_str + shadow_delim + std::to_string(shadow_count);
        }

        V8MBLContext JSContext::HostCreateShadowRealmContext(V8LContext inInitiator)
        {
            // TODO: rework this to create the approriate shadow context
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inInitiator);
            if (jsContext == nullptr)
            {
                return V8MBLContext();
            }
            std::string shadowName = jsContext->GenerateShadowName();
            JSContextSharedPtr shadow = jsContext->m_Runtime->CreateContext(shadowName, "");
            V8LContext shadow_local = shadow->GetLocalContext();
            shadow_local->SetSecurityToken(inInitiator->GetSecurityToken());

            return jsContext->GetLocalContext();
        }

        void JSContext::MoveContext(JSContext &&inContext)
        {
            // need to get the wekref first before we move anything
            JSContextWeakPtr *weakRef = inContext.GetContextWeakRef();
            m_Runtime = inContext.m_Runtime;
            inContext.m_Runtime = nullptr;

            m_Initialized = inContext.m_Initialized;
            inContext.m_Initialized = false;

            m_Context = std::move(inContext.m_Context);
            *weakRef = shared_from_this();
            inContext.m_Context.Reset();

            m_Modules = inContext.m_Modules;
            inContext.m_Modules.reset();

            m_Name = inContext.m_Name;
            m_SnapIndex = inContext.m_SnapIndex;
            m_Namespace = inContext.m_Namespace;
            m_EntryPoint = inContext.m_EntryPoint;
            m_SupportsSnapshots = inContext.m_SupportsSnapshots;
            m_SnapMethod = inContext.m_SnapMethod;
        }

        bool JSContext::CreateContext()
        {
            IJSSnapshotProviderSharedPtr provider = GetJSApp()->GetSnapshotProvider();
            if (provider == nullptr)
            {
                LOG_ERROR("The Snapshot provider doesn't appear to be set yet");
                return false;
            }
            V8Isolate *isolate = m_Runtime->GetIsolate();
            if (isolate == nullptr)
            {
                LOG_ERROR("THe Runtime doesn't seemed to be initialized as the isolate is null");
                return false;
            }

            m_Modules = std::make_shared<JSContextModules>(shared_from_this());

            V8IsolateScope isolateScope(isolate);
            V8TryCatch tryCatch(isolate);
            V8HandleScope handleScope(isolate);

            V8LContext context;

            if (m_SnapIndex == 0)
            {
                context = V8Context::New(isolate, nullptr, {});

                if (context.IsEmpty())
                {
                    LOG_ERROR("Failed to create a new V8 Context");
                    return false;
                }

                m_Context.Reset(isolate, context);

                // Set the security token for shadow realms.
                // TODO: figure out how to determine if a context from a snapshot keeps or needs this reset
                uuids::uuid uuid = uuids::uuid_system_generator{}();
                m_SecurityToken = uuids::to_string(uuid);
                V8LString v8Uuid = JSUtilities::StringToV8(isolate, m_SecurityToken);
                context->SetSecurityToken(v8Uuid);
                m_Runtime->RegisterNamespaceFunctionsOnGlobal(m_Namespace, context, context->Global());

                AttachWeakPtr(context);
            }
            else
            {
                size_t realContextIndex = provider->RealContextIndex(m_SnapIndex);
                // Coming from a snapshot we don't have to create the global template
                // TODO: Look at removing the ToLocalChecked in favor oof ToLocal so we
                // don't assert if it fails and can handle it ourselves
                context = V8Context::FromSnapshot(isolate,
                                                  realContextIndex,
                                                  provider->GetInternalDeserializerCallback(),
                                                  nullptr,
                                                  V8MBLValue(),
                                                  nullptr,
                                                  provider->GetContextDeserializerCallaback(this),
                                                  provider->GetAPIWrapperDeserializerCallback())
                              .ToLocalChecked();
                m_Context.Reset(isolate, context);

                V8LString token = context->GetSecurityToken().As<V8String>();
                m_SecurityToken = JSUtilities::V8ToString(isolate, token);
                // The weak ptr gets attached during deserialization
                JSRuntimeSnapDataSharedPtr snapData = m_Runtime->GetRuntimeSnapData();
                if (snapData == nullptr)
                {
                    LOG_ERROR("Snapshot provider returned a null JSRuntimeSnapData");
                    return false;
                }
                if (realContextIndex >= snapData->m_ContextData.size())
                {
                    LOG_ERROR("M_SnapIndex is out of the context snap data");
                    return false;
                }
                if (m_Modules->RestoreModules(snapData->m_ContextData[realContextIndex]->m_Modules) == false)
                {
                    return false;
                }
            }

            if (tryCatch.HasCaught())
            {
                Log::LogMessage msg{
                    {Log::MsgKey::Msg, "Got an error trying to create ht econtext"},
                    {Log::MsgKey::StackTrace, JSUtilities::GetStackTrace(isolate, tryCatch)}};
                LOG_ERROR(msg);
                return false;
            }

            m_Initialized = true;

            if (m_Runtime->IsSnapshotRuntime())
            {
                m_Runtime->RegisterSnapshotHandleCloser(this);
            }

            return true;
        }

        void JSContext::AttachWeakPtr(V8LContext inContext)
        {
            JSContextWeakPtr *weakPtr = new JSContextWeakPtr(weak_from_this());
            inContext->SetAlignedPointerInEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr), weakPtr);
        }

        V8LValue JSContext::RunModule(std::filesystem::path inModulePath)
        {
            V8Isolate *isolate = m_Runtime->GetIsolate();
            JSModuleInfoSharedPtr module = m_Modules->LoadModule(inModulePath);
            if (module == nullptr)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to load the module: {}", inModulePath));
                return V8LValue();
            }
            if (m_Modules->InstantiateModule(module) == false)
            {
                JSUtilities::ThrowV8Error(isolate, JSUtilities::V8Errors::Error, Utils::format("Failed to instantiate the module: {}", inModulePath));
                return V8LValue();
            }
            return m_Modules->RunModule(module);
        }

        V8LValue JSContext::RunScript(std::string inScript)
        {
            V8Isolate *isolate = m_Runtime->GetIsolate();

            V8IsolateScope iScope(isolate);
            v8::EscapableHandleScope eScope(isolate);
            V8LContext v8Context = GetLocalContext();
            V8ContextScope cScope(v8Context);

            V8TryCatch tryCatch(isolate);

            V8LString source = JSUtilities::StringToV8(isolate, inScript.c_str());
            if (tryCatch.HasCaught())
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, JSUtilities::GetStackTrace(isolate, tryCatch));
                LOG_ERROR(msg);
                tryCatch.ReThrow();
                return V8LValue();
            }

            V8MLScript maybeScript = v8::Script::Compile(v8Context, source);
            if (tryCatch.HasCaught())
            {
                LOG_ERROR(JSUtilities::GetStackTrace(isolate, tryCatch));
                tryCatch.ReThrow();
                return V8LValue();
            }

            V8LScript script;
            script = maybeScript.FromMaybe(V8LScript());
            if (script.IsEmpty())
            {
                LOG_ERROR(JSUtilities::GetStackTrace(isolate, tryCatch));
                tryCatch.ReThrow();
                return V8LValue();
            }

            V8LValue result;
            if (script->Run(v8Context).ToLocal(&result) == false)
            {
                LOG_ERROR(JSUtilities::GetStackTrace(isolate, tryCatch));
                tryCatch.ReThrow();
                return V8LValue();
            }
            return eScope.Escape(result);
        }

        void JSContext::SerializeContextData(Serialization::WriteBuffer &inBuffer)
        {
            inBuffer << m_Name;
            inBuffer << m_Namespace;
        }

        void JSContext::DeserializeContextData(V8LContext inContext, Serialization::ReadBuffer &inBuffer)
        {
            AttachWeakPtr(inContext);
            inBuffer >> m_Name;
            inBuffer >> m_Namespace;
        }

        bool JSContext::MakeSnapshot(V8SnapshotCreatorSharedPtr inCreator, v8App::Serialization::WriteBuffer &inBuffer)
        {
            inBuffer << m_Name;
            inBuffer << m_Namespace;
            inBuffer << m_EntryPoint.string();
            if (inBuffer.HasErrored())
            {
                return false;
            }
            return m_Modules->MakeSnapshot(inCreator, inBuffer);
        }

        JSContextSnapDataSharedPtr JSContext::LoadSnapshotData(v8App::Serialization::ReadBuffer &inBuffer)
        {
            JSContextSnapDataSharedPtr data = std::make_shared<JSContextSnapData>();
            inBuffer >> data->m_Name;
            inBuffer >> data->m_Namespace;
            inBuffer >> data->m_EntryPoint;
            if (inBuffer.HasErrored())
            {
                return nullptr;
            }
            m_Modules = std::make_shared<JSContextModules>(shared_from_this());
            if (m_Modules->LoadSnapshotData(inBuffer, *data) == false)
            {
                return nullptr;
            }
            return data;
        }

        V8LValue JSContext::RunEntryPoint(bool inSnapshotting)
        {
            if (m_EntryPoint == "")
            {
                return V8LValue();
            }

            return RunModule(m_EntryPoint);
        }

        JSContextSharedPtr JSContext::CloneForSnapshot(JSRuntimeSharedPtr inRuntime)
        {
            JSContextSharedPtr context = std::make_shared<JSContext>(inRuntime, m_Name, m_Namespace,
                                                                     m_EntryPoint, m_SnapIndex,
                                                                     m_SupportsSnapshots, m_SnapMethod);
            // create it's context
            if (context->CreateContext() == false)
            {
                return nullptr;
            }
            // if the context is not 0 then there is no need to set it up
            if (m_SnapIndex != 0)
            {
                return context;
            }

            V8IsolateScope iScope(inRuntime->GetIsolate());
            V8HandleScope hScope(inRuntime->GetIsolate());
            V8LContext v8Context = context->GetLocalContext();
            V8ContextScope cScope(v8Context);

            // if we only are doing the namespace then skip the script execution
            if (m_SnapMethod == SnapshotMethod::kNamespaceOnly)
            {
                return context;
            }

            if (context->RunEntryPoint(true).IsEmpty())
            {
                return nullptr;
            }
            return context;
        }

        void JSContext::CloseHandleForSnapshot()
        {
            DisposeV8Context(true);
        }

        void JSContext::DisposeContext()
        {
            if (m_Initialized == false)
            {
                return;
            }
            if (m_Runtime != nullptr)
            {
                m_Runtime->UnregisterSnapshotHandlerCloser(this);
            }
            DisposeV8Context();

            m_Runtime = nullptr;
            m_Initialized = false;
        }

        JSContextWeakPtr *JSContext::GetContextWeakRef()
        {
            V8Isolate *isolate = GetIsolate();
            if (isolate == nullptr || m_Context.IsEmpty())
            {
                return nullptr;
            }
            V8LContext context = m_Context.Get(isolate);
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return nullptr;
            }
            return weakPtr;
        }
    }
}
