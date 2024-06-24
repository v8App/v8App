// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <algorithm>

#include "uuid/uuid.h"

#include "v8/v8.h"
#include "Logging/LogMacros.h"
#include "JSContext.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContext::JSContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
                             std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot,
                             SnapshotMethod inSnapMethod)
            : m_Runtime(inRuntime), m_Name(inName), m_Namespace(inNamespace), m_EntryPoint(inEntryPoint),
              m_SnapEntryPoint(inSnapEntryPoint), m_m_SupportsSnapshots(inSupportsSnapshot), m_SnapMethod(inSnapMethod)
        {
            CHECK_NE(m_Runtime.get(), nullptr);
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

        v8::Isolate *JSContext::GetIsolate()
        {
            return (m_Runtime == nullptr) ? nullptr : m_Runtime->GetIsolate();
        }

        JSAppSharedPtr JSContext::GetJSApp()
        {
            return (m_Runtime == nullptr) ? nullptr : m_Runtime->GetApp();
        }

        JSContextSharedPtr JSContext::GetJSContextFromV8Context(V8LocalContext inContext)
        {
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(inContext->GetAlignedPointerFromEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return JSContextSharedPtr();
            }
            return weakPtr->lock();
        }

        V8LocalContext JSContext::GetLocalContext()
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr)
            {
                return V8LocalContext();
            }
            return m_Context.Get(isolate);
        }

        void JSContext::SetupShadowRealmCallback(V8Isolate *inIsolate)
        {
            inIsolate->SetHostCreateShadowRealmContextCallback(JSContext::HostCreateShadowRealmContext);
        }

        void JSContext::DisposeV8Context(bool forSnapshot)
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate != nullptr)
            {
                if (m_Context.IsEmpty() == false)
                {
                    v8::Local<v8::Context> context = m_Context.Get(isolate);

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

        V8MaybeLocalContext JSContext::HostCreateShadowRealmContext(V8LocalContext inInitiator)
        {
            //TODO: rework this to create the approriate shadow context
            JSContextSharedPtr jsContext = JSContext::GetJSContextFromV8Context(inInitiator);
            if (jsContext == nullptr)
            {
                return V8MaybeLocalContext();
            }
            std::string shadowName = jsContext->GenerateShadowName();
            JSContextSharedPtr shadow = jsContext->m_Runtime->CreateContext(shadowName, "");
            V8LocalContext shadow_local = shadow->GetLocalContext();
            shadow_local->SetSecurityToken(inInitiator->GetSecurityToken());

            return jsContext->GetLocalContext();
        }

        void JSContext::MoveContext(JSContext &&inContext)
        {
            // need to get teh wekref first before we move anything
            JSContextWeakPtr *weakRef = inContext.GetContextWeakRef();
            m_Runtime = inContext.m_Runtime;
            inContext.m_Runtime = nullptr;

            m_Initialized = std::move(inContext.m_Initialized);
            inContext.m_Initialized = false;

            m_Context = std::move(inContext.m_Context);
            *weakRef = shared_from_this();
            inContext.m_Context.Reset();

            m_Modules = inContext.m_Modules;
            inContext.m_Modules.reset();
        }

        bool JSContext::CreateContext(size_t inSnapIndex)
        {
            V8SnapshotProviderSharedPtr provider = m_Runtime->GetApp()->GetSnapshotProvider();
            if (provider == nullptr)
            {
                return false;
            }
            v8::Isolate *isolate = m_Runtime->GetIsolate();
            if (isolate == nullptr)
            {
                return false;
            }

            m_Index = inSnapIndex;

            v8::Isolate::Scope isolateScope(isolate);
            v8::TryCatch tryCatch(isolate);
            v8::HandleScope handleScope(isolate);

            V8LocalContext context;

            if (m_Index == 0)
            {
                context = v8::Context::New(isolate, nullptr, {});

                if (context.IsEmpty())
                {
                    return false;
                }

                // Set the security token for shadow realms.
                // TODO: figure out how to determine if context form a snapshot keeps or needs this reset
                uuids::uuid uuid = uuids::uuid_system_generator{}();
                V8LocalString v8Uuid = JSUtilities::StringToV8(isolate, uuids::to_string(uuid));
                context->SetSecurityToken(v8Uuid);
            }
            else
            {
                // Coming from a snapshot we don't have to create the global template
                context = v8::Context::FromSnapshot(isolate,
                                                    m_Index,
                                                    provider->GetInternalDeserializerCallback(),
                                                    nullptr,
                                                    v8::MaybeLocal<v8::Value>(),
                                                    nullptr,
                                                    provider->GetContextDeserializerCallaback())
                              .ToLocalChecked();
            }

            DCHECK_FALSE(tryCatch.HasCaught());

            JSContextWeakPtr *weakPtr = new JSContextWeakPtr(weak_from_this());
            context->SetAlignedPointerInEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr), weakPtr);
            m_Modules = std::make_shared<JSContextModules>(shared_from_this());

            m_Context.Reset(isolate, context);
            m_Initialized = true;
            return true;
        }

        void JSContext::RegisterSnapshotCloser()
        {
            if (m_Runtime != nullptr)
            {
                m_Runtime->RegisterSnapshotHandleCloser(shared_from_this());
            }
        }

        void JSContext::UnregisterSnapshotCloser()
        {
            if (m_Runtime != nullptr)
            {
                m_Runtime->UnregisterSnapshotHandlerCloser(shared_from_this());
            }
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
            DisposeV8Context();

            m_Runtime = nullptr;
            m_Initialized = false;
        }

        JSContextWeakPtr *JSContext::GetContextWeakRef()
        {
            v8::Isolate *isolate = GetIsolate();
            if (isolate == nullptr || m_Context.IsEmpty())
            {
                return nullptr;
            }
            v8::Local<v8::Context> context = m_Context.Get(isolate);
            JSContextWeakPtr *weakPtr = static_cast<JSContextWeakPtr *>(context->GetAlignedPointerFromEmbedderData(int(JSContext::DataSlot::kJSContextWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return nullptr;
            }
            return weakPtr;
        }
    }
}
