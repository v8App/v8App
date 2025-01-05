// Copyright 2020 the v8App authors. All right reserved.
// Use of this source code is governed by the MIT license
// that can be found in the LICENSE file.

#include "v8/v8-cppgc.h"

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "Serialization/ReadBuffer.h"
#include "Serialization/TypeSerializer.h"
#include "Utils/Format.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "JSContextModules.h"
#include "ForegroundTaskRunner.h"
#include "CppBridge/CallbackRegistry.h"
#include "JSContext.h"
#include "V8Types.h"
#include "V8AppPlatform.h"
#include "IJSSnapshotCreator.h"
#include "IJSSnapshotProvider.h"
#include "JSRuntimeSnapData.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntime::JSRuntime()
        {
        }

        JSRuntime::~JSRuntime()
        {
            DisposeRuntime();
        }

        JSRuntime::JSRuntime(JSRuntime &&inRuntime)
        {
            m_App = std::move(inRuntime.m_App);
            m_IdleEnabled = inRuntime.m_IdleEnabled;
            m_Name = inRuntime.m_Name;
            m_HandleClosers = std::move(inRuntime.m_HandleClosers);
            m_Isolate = std::move(inRuntime.m_Isolate);
            m_Contextes = std::move(inRuntime.m_Contextes);
            m_TaskRunner = std::move(inRuntime.m_TaskRunner);
            m_ObjectTemplates = std::move(inRuntime.m_ObjectTemplates);
            m_Creator = std::move(inRuntime.m_Creator);
            m_IsSnapshotter = inRuntime.m_IsSnapshotter;

            m_Initialized = inRuntime.m_Initialized;
            inRuntime.m_Initialized = false;
        }

        bool JSRuntime::Initialize(JSAppSharedPtr inApp, std::string inName, size_t inRuntimeIndex, JSRuntimeSnapshotAttributes inSnapAttribute,
                                   bool isSnapshottable, IdleTaskSupport inEnableIdle)
        {
            if (m_Initialized)
            {
                return true;
            }

            if (inApp == nullptr)
            {
                LOG_ERROR("JSApp was a nullptr");
                return false;
            }

            m_Name = inName;
            m_App = inApp;
            m_TaskRunner = std::make_shared<ForegroundTaskRunner>();
            m_IsSnapshotter = isSnapshottable;
            m_Snapshottable = inSnapAttribute;
            m_IdleEnabled = inEnableIdle;
            m_SnapshotIndex = inRuntimeIndex;

            if (CreateIsolate() == false)
            {
                return false;
            }

            JSContextModules::SetupModulesCallbacks(m_Isolate.get());
            m_Initialized = true;
            return true;
        }

        JSRuntimeSharedPtr JSRuntime::GetJSRuntimeFromV8Isolate(V8Isolate *inIsloate)
        {
            if (inIsloate == nullptr)
            {
                return nullptr;
            }
            JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(inIsloate->GetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr)));
            if (weakPtr == nullptr || weakPtr->expired())
            {
                return JSRuntimeSharedPtr();
            }
            return weakPtr->lock();
        }

        void JSRuntime::ProcessTasks()
        {
            while (m_TaskRunner->MaybeHasTask())
            {
                V8TaskUniquePtr task = m_TaskRunner->GetNextTask();
                {
                    V8IsolateScope isolateScope(m_Isolate.get());
                    V8Locker locker(m_Isolate.get());
                    ForegroundTaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run();
                }
            }
        }

        void JSRuntime::ProcessIdleTasks(double inTimeLeft)
        {
            if (IdleTasksEnabled() == false)
            {
                return;
            }

            double deadline = Time::MonotonicallyIncreasingTimeSeconds() + inTimeLeft;

            while (deadline > Time::MonotonicallyIncreasingTimeSeconds() && m_TaskRunner->MaybeHasIdleTask())
            {
                V8IdleTaskUniquePtr task = m_TaskRunner->GetNextIdleTask();
                {
                    V8IsolateScope isolateScope(m_Isolate.get());
                    V8Locker locker(m_Isolate.get());
                    ForegroundTaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run(deadline);
                }
            }
        }

        void JSRuntime::SetFunctionTemplate(std::string inJSFuncName, v8::Local<V8FuncTpl> inTemplate, std::string inNamespace)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            SetFunctionTemplateInternal(inJSFuncName, inTemplate, inNamespace, nullptr);
        }

        void JSRuntime::SetClassFunctionTemplate(std::string inNamespace, CppBridge::V8CppObjInfo *inObjInfo, v8::Local<V8FuncTpl> inTemplate)
        {
            CHECK_NOT_NULL(m_Isolate.get());

            // if they pass an empty string make it global
            if (inNamespace == "")
            {
                inNamespace = "global";
            }
            FunctionTemplateInfoSharedPtr info = SetFunctionTemplateInternal(inObjInfo->m_JsClassName, inTemplate, inNamespace, inObjInfo);
            ObjectTemplateMap::iterator it = m_ObjectTemplates.find(inObjInfo);
            if (it == m_ObjectTemplates.end())
            {
                m_ObjectTemplates[inObjInfo] = info;
            }
        }

        v8::Local<V8FuncTpl> JSRuntime::GetClassFunctionTemplate(CppBridge::V8CppObjInfo *inInfo)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            ObjectTemplateMap::iterator it = m_ObjectTemplates.find(inInfo);
            if (it == m_ObjectTemplates.end())
            {
                return v8::Local<V8FuncTpl>();
            }
            return it->second->m_Template.Get(m_Isolate.get());
        }

        void JSRuntime::RegisterNamespaceFunctionsOnGlobal(std::string inNamespace, V8LContext inContext, V8LObject inGlobal)
        {
            if (inNamespace.empty())
            {
                inNamespace = "global";
            }
            for (auto const &it : m_FunctinNameMap)
            {
                // register all the global function templates and ones that match the passed in namespace
                if (it.second->m_Namespace != inNamespace && it.second->m_Namespace != "global")
                {
                    continue;
                }
                V8LFuncTpl tpl = it.second->m_Template.Get(m_Isolate.get());
                inGlobal->Set(inContext,
                              JSUtilities::StringToV8(m_Isolate.get(), it.second->m_FunctionName),
                              tpl->GetFunction(inContext).ToLocalChecked());
            }
        }

        JSContextSharedPtr JSRuntime::CreateContext(std::string inName, std::filesystem::path inEntryPoint, std::string inNamespace,
                                                    bool inSupportsSnapshot, SnapshotMethod inSnapMethod)
        {
            if (GetContextProvider() == nullptr || m_App->GetSnapshotProvider() == nullptr)
            {
                return nullptr;
            }
            JSContextSharedPtr context = GetContextProvider()->CreateContext(shared_from_this(), inName,
                                                                             inNamespace, inEntryPoint,
                                                                             inSupportsSnapshot, inSnapMethod, 0, true);
            if (context == nullptr)
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context");
                Log::Log::Error(message);
            }
            else
            {
                m_Contextes.insert(std::make_pair(inName, context));
            }
            return context;
        }

        JSContextSharedPtr JSRuntime::CreateContextFromSnapshot(std::string inName, std::string inIndexName, bool inSupportsSnapshot,
                                                                SnapshotMethod inSnapMethod)
        {
            if (m_App->GetSnapshotProvider() == nullptr)
            {
                LOG_ERROR("SnapshotProvider hasn't been set");
                return nullptr;
            }

            size_t contextIndex = m_App->GetSnapshotProvider()->GetIndexForContextName(inIndexName, m_SnapshotIndex);
            if (m_App->GetSnapshotProvider()->IsContextIndexValid(contextIndex, m_SnapshotIndex) == false)
            {
                Log::Log::Error(Utils::format("ContextHelper Says that the intext Index {} is invalid", inIndexName));
                return nullptr;
            }
            return CreateContextFromSnapshot(inName, contextIndex, inSupportsSnapshot, inSnapMethod);
        }

        JSContextSharedPtr JSRuntime::CreateContextFromSnapshot(std::string inName, size_t inIndex, bool inSupportsSnapshot,
                                                                SnapshotMethod inSnapMethod)
        {
            if (GetContextProvider() == nullptr || m_App->GetSnapshotProvider() == nullptr)
            {
                LOG_ERROR("ContextProvider helpr or SnapshotProvider hasn't been set");
                return nullptr;
            }
            if (m_App->GetSnapshotProvider()->IsContextIndexValid(inIndex, m_SnapshotIndex) == false)
            {
                LOG_ERROR(Utils::format("ContextHelper Says that the intext Index {} is invalid", inIndex));
                return nullptr;
            }
            JSContextSharedPtr context = GetContextProvider()->CreateContext(shared_from_this(), inName, "", "",
                                                                             inSupportsSnapshot, inSnapMethod, inIndex, false);
            if (context == nullptr)
            {
                Log::Log::Error("ContextHelper returned a nullptr for the context");
            }
            else
            {
                m_Contextes.insert(std::make_pair(inName, context));
            }
            return context;
        }

        JSContextSharedPtr JSRuntime::GetContextByName(std::string inName)
        {
            auto it = m_Contextes.find(inName);
            if (it == m_Contextes.end())
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, Utils::format("Failed to find JSContext with name {}", inName));
                Log::Log::Warn(message);

                return JSContextSharedPtr();
            }
            return it->second;
        }

        void JSRuntime::DisposeContext(std::string inName)
        {
            JSContextSharedPtr context = GetContextByName(inName);
            DisposeContext(context);
        }

        void JSRuntime::DisposeContext(JSContextSharedPtr inContext)
        {
            if (inContext == nullptr)
            {
                return;
            }

            if (m_App->GetContextProvider() == nullptr)
            {
                return;
            }

            // m_ContextProvider->UnregisterSnapshotCloser(inContext);

            m_App->GetContextProvider()->DisposeContext(inContext);
            for (auto it = m_Contextes.begin(); it != m_Contextes.end(); it++)
            {
                if (it->second == inContext)
                {
                    m_Contextes.erase(it);
                    return;
                }
            }
        }

        void JSRuntime::DisposeRuntime()
        {
            if (m_Initialized == false)
            {
                return;
            }
            m_HandleClosers.clear();
            if (m_Isolate != nullptr && m_Creator == nullptr)
            {
                V8IsolateScope isolateScope(m_Isolate.get());
                V8Locker locker(m_Isolate.get());
                V8HandleScope handleScope(m_Isolate.get());
                for (auto &it : m_FunctinNameMap)
                {
                    it.second->m_Template.Reset();
                }
                m_FunctinNameMap.clear();
                m_ObjectTemplates.clear();

                m_Contextes.clear();
                JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(m_Isolate->GetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr)));
                delete weakPtr;
                m_Isolate->SetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr), nullptr);
            }
            m_Creator.reset();
            m_Isolate.reset();
            m_App.reset();
            m_TaskRunner.reset();
            m_Initialized = false;
        }

        IJSContextProviderSharedPtr JSRuntime::GetContextProvider()
        {
            if (m_CustomContextProvider != nullptr || m_App == nullptr)
            {
                return m_CustomContextProvider;
            }
            return m_App->GetContextProvider();
        }

        void JSRuntime::SetContextProvider(IJSContextProviderSharedPtr inProvider)
        {
            // unlike in the app we allow being able to set it back to nullptr
            // to revert back to the app
            m_CustomContextProvider = inProvider;
        }

        bool JSRuntime::MakeSnapshot(Serialization::WriteBuffer &inBuffer, void *inData)
        {
            inBuffer << m_Name;
            inBuffer << m_IdleEnabled;
            inBuffer << m_Snapshottable;

            Containers::NamedIndexes contextIndexes;
            IJSSnapshotCreatorSharedPtr snapCreator = m_App->GetSnapshotCreator();

            {
                V8IsolateScope iScope(m_Isolate.get());
                V8HandleScope hiScope(m_Isolate.get());
                {
                    V8LContext defaultContext = V8Context::New(m_Isolate.get());

                    // we always add a v8 context to the snapshot so nothing is ever assigned
                    m_Creator->SetDefaultContext(defaultContext, snapCreator->GetInternalSerializerCallaback(),
                                                 snapCreator->GetContextSerializerCallback(), snapCreator->GetAPIWrapperSerializerCallaback());
                }

                inBuffer << m_Contextes.size();
                for (auto [name, context] : m_Contextes)
                {
                    if (context->SupportsSnapshots() == false)
                    {
                        continue;
                    }

                    V8LContext v8Context = context->GetLocalContext();
                    if (context->MakeSnapshot(m_Creator, inBuffer) == false)
                    {
                        return false;
                    }
                    size_t index = m_Creator->AddContext(v8Context, snapCreator->GetInternalSerializerCallaback(),
                                                         snapCreator->GetContextSerializerCallback(), snapCreator->GetAPIWrapperSerializerCallaback());
                    if (contextIndexes.AddNamedIndex(index, name) == false)
                    {
                        LOG_ERROR(Utils::format("Failed to add the context {} to the named indexes", name));
                        return false;
                    }
                }

                inBuffer << contextIndexes;
                if (inBuffer.HasErrored())
                {
                    LOG_ERROR(Utils::format("Failed to seriaize the context indexes for runtime {}", m_Name));
                    return false;
                }

                // Save of the function templates
                inBuffer << m_FunctinNameMap.size();
                for (auto it : m_FunctinNameMap)
                {
                    JSRuntimeSnapData::FuncTplSnapshotData snapData;
                    snapData.m_Namespace = it.second->m_Namespace;
                    snapData.m_FunctionName = it.second->m_FunctionName;
                    if (it.second->m_ObjInfo != nullptr)
                    {
                        snapData.m_ClassName = it.second->m_ObjInfo->m_TypeName;
                    }
                    snapData.m_IsolateDataIndex = m_Creator->AddData(it.second->m_Template.Get(m_Isolate.get()));
                    inBuffer << snapData;
                }

                CloseOpenHandlesForSnapshot();
                V8StartupData snapshot = m_Creator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);
                if (snapshot.data == nullptr)
                {
                    LOG_ERROR(Utils::format("Failed to create the snapshot for runtime {}", m_Name));
                    return false;
                }

                inBuffer << snapshot.raw_size;
                inBuffer.SerializeWrite(snapshot.data, snapshot.raw_size);
            }
            return true;
        }

        JSRuntimeSnapDataSharedPtr JSRuntime::LoadSnapshotData(Serialization::ReadBuffer &inBuffer)
        {
            JSRuntimeSnapDataSharedPtr snapData = CreateSnapData();

            inBuffer >> snapData->m_RuntimeName;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading runtime name");
                return {};
            }
            inBuffer >> snapData->m_IdleEnabled;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading runtime idle enabled");
                return {};
            }
            inBuffer >> snapData->m_SnashotAttribute;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading snapshot attribute");
                return {};
            }
            size_t numContextes;
            inBuffer >> numContextes;
            for (size_t idx = 0; idx < numContextes; idx++)
            {
                JSContextSharedPtr context = std::make_shared<JSContext>();
                JSContextSnapDataSharedPtr data = context->LoadSnapshotData(inBuffer);
                if (data == nullptr)
                {
                    return {};
                }
                snapData->m_ContextData.push_back(data);
            }

            inBuffer >> snapData->m_ContextIndexes;
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading runtime context namespaces");
                return {};
            }

            size_t numFuncTemplates;
            inBuffer >> numFuncTemplates;
            for (size_t idx = 0; idx < numFuncTemplates; idx++)
            {
                JSRuntimeSnapData::FuncTplSnapshotData funcSnapData;
                inBuffer >> funcSnapData;
                if (inBuffer.HasErrored())
                {
                    LOG_ERROR("Buffer read failed reading the function template info");
                    return {};
                }
                snapData->m_FunctionTemplates.push_back(funcSnapData);
            }

            int dataSize;
            inBuffer >> dataSize;

            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading runtime isolate data size");
                return {};
            }
            snapData->m_StartupData.raw_size = dataSize;
            snapData->m_StartupDeleter = std::make_unique<char[]>(dataSize);
            inBuffer.SerializeRead(snapData->m_StartupDeleter.get(), dataSize);
            if (inBuffer.HasErrored())
            {
                LOG_ERROR("Buffer read failed on reading isoalte data");
                return {};
            }
            snapData->m_StartupData.data = snapData->m_StartupDeleter.get();

            return snapData;
        }

        bool JSRuntime::RestoreSnapshot(JSAppSharedPtr inApp, JSRuntimeSnapDataSharedPtr inSnapData, size_t inSnapIndex)
        {
            if (m_Initialized)
            {
                return true;
            }

            m_Name = inSnapData->m_RuntimeName;
            m_App = inApp;
            m_TaskRunner = std::make_shared<ForegroundTaskRunner>();
            m_Snapshottable = inSnapData->m_SnashotAttribute;
            m_IdleEnabled = inSnapData->m_IdleEnabled;
            m_SnapshotIndex = inSnapIndex;
            // Rstored Runtimes are never snapshottable
            m_IsSnapshotter = false;
            if (CreateIsolate() == false)
            {
                return false;
            }
            JSContextModules::SetupModulesCallbacks(m_Isolate.get());
            //at this point we the runtime is initialized
            m_Initialized = true;

            for (size_t idx = 0; idx < inSnapData->m_ContextData.size(); idx++)
            {
                JSContextSnapDataSharedPtr ctxSnapData = inSnapData->m_ContextData[idx];
                IJSContextProviderSharedPtr provider = GetContextProvider();
                if (provider == nullptr)
                {
                    return false;
                }
                JSContextSharedPtr jsContext = provider->CreateContext(
                    shared_from_this(),
                    ctxSnapData->m_Name,
                    ctxSnapData->m_Namespace,
                    "",
                    false,
                    SnapshotMethod::kNamespaceOnly,
                    idx,
                    false);
                if (jsContext == nullptr)
                {
                    return false;
                }
                m_Contextes.insert(std::make_pair(jsContext->GetName(), jsContext));
            }
            
            return true;
        }

        JSRuntimeSnapDataSharedPtr JSRuntime::GetRuntimeSnapData()
        {
            if (m_Initialized == false)
            {
                return nullptr;
            }
            return GetRuntimeSnapDataInternal();
        }

        void JSRuntime::CloseOpenHandlesForSnapshot()
        {
            // If not a snapshotting runtime return
            if (m_IsSnapshotter == false)
            {
                return;
            }

            for (auto callback : m_HandleClosers)
            {
                callback->CloseHandleForSnapshot();
            }
            // clear the callbacks since they are all called
            m_HandleClosers.clear();
        }

        void JSRuntime::RegisterSnapshotHandleCloser(ISnapshotHandleCloser *inCloser)
        {
            // not a snapshotter no reason to register them
            if (m_IsSnapshotter == false)
            {
                return;
            }
            // no point in adding an expired ptr
            if (inCloser == nullptr)
            {
                return;
            }
            if (std::find(m_HandleClosers.begin(), m_HandleClosers.end(), inCloser) != m_HandleClosers.end())
            {
                return;
            }
            m_HandleClosers.push_back(inCloser);
        }

        void JSRuntime::UnregisterSnapshotHandlerCloser(ISnapshotHandleCloser *inCloser)
        {
            // not a snapshotter no reason to unregister them
            if (m_IsSnapshotter == false)
            {
                return;
            }

            // if empty no need to go on
            if (m_HandleClosers.empty())
            {
                return;
            }

            // we loop through the callbacks to find the registered callback but
            // also any callabcks that are expired just to clean them out
            auto it = std::find(m_HandleClosers.begin(), m_HandleClosers.end(), inCloser);
            std::vector<ISnapshotHandleCloser *> removePos;
            if (it == m_HandleClosers.end())
            {
                return;
            }
            m_HandleClosers.erase(it);
        }

        V8CppHeap *JSRuntime::GetCppHeap()
        {
            if (m_Isolate == nullptr)
            {
                return nullptr;
            }
            return m_Isolate->GetCppHeap();
        }

        JSRuntime::FunctionTemplateInfoSharedPtr JSRuntime::SetFunctionTemplateInternal(std::string inJSFuncName, v8::Local<V8FuncTpl> inTemplate, std::string inNamespace, CppBridge::V8CppObjInfo *inObjInfo)
        {
            if (inNamespace.empty() == false)
            {
                inNamespace = "global";
            }
            std::string fullName = inNamespace + "." + inJSFuncName;
            FunctionNameMap::iterator it = m_FunctinNameMap.find(fullName);
            if (it == m_FunctinNameMap.end())
            {
                FunctionTemplateInfoSharedPtr info = std::make_shared<FunctionTemplateInfo>();
                info->m_FunctionName = inJSFuncName;
                info->m_Namespace = inNamespace;
                info->m_Template.Reset(m_Isolate.get(), inTemplate);
                info->m_ObjInfo = inObjInfo;
                m_FunctinNameMap[fullName] = info;
                return info;
            }
            else
            {
                if (it->second->m_Template != inTemplate)
                {
                    LOG_WARN(Utils::format("{} has lready been registerd with a different function template", inJSFuncName));
                }
                return it->second;
            }
        }

        JSRuntimeSnapDataSharedPtr JSRuntime::GetRuntimeSnapDataInternal()
        {
            JSAppSnapDataSharedPtr snapData = m_App->GetSnapshotProvider()->GetJSAppSnapData();
            if (snapData == nullptr)
            {
                return nullptr;
            }
            return snapData->m_RuntimesSnapData[m_SnapshotIndex];
        }

        std::string JSRuntime::ResolveContextName(std::string inName, std::string inNamespace, SnapshotMethod inMethod)
        {
            if (inMethod == SnapshotMethod::kNamespaceOnly)
            {
                return inNamespace;
            }
            return inNamespace + "::" + inName;
        }

        bool JSRuntime::CreateIsolate()
        {
            if (m_App->GetSnapshotProvider() == nullptr)
            {
                return false;
            }
            V8Isolate::CreateParams params;
            params.snapshot_blob = m_App->GetSnapshotProvider()->GetSnapshotData(m_SnapshotIndex);
            if (params.snapshot_blob == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to get the snapshot data for index {} form the snapshot provider", m_SnapshotIndex));
                return false;
            }
            params.external_references = m_App->GetSnapshotProvider()->GetExternalReferences();

            // TODO: replace with custom allocator
            params.array_buffer_allocator =
                V8ArrayBuffer::Allocator::NewDefaultAllocator();

            V8CppHeapUniquePtr heap = V8CppHeap::Create(V8AppPlatform::Get().get(), v8::CppHeapCreateParams({}));
            params.cpp_heap = heap.get();
            // the isolate will own the heap so release it
            heap.release();

            // custom deleter since we have to call dispose
            V8Isolate *isolate = V8Isolate::Allocate();

            m_Isolate = std::shared_ptr<V8Isolate>(isolate, [](V8Isolate *isolate)
                                                   { isolate->Dispose(); });
            m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true);
            JSRuntimeWeakPtr *weakPtr = new JSRuntimeWeakPtr(shared_from_this());
            m_Isolate->SetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr), weakPtr);

            if (m_IsSnapshotter)
            {
                m_Creator = std::make_unique<V8SnapCreator>(isolate, params);
            }
            else
            {
                V8Isolate::Initialize(isolate, params);
                JSRuntimeSnapDataSharedPtr snapData = GetRuntimeSnapDataInternal();
                if (snapData != nullptr && snapData->m_FunctionTemplates.size())
                {
                    V8Isolate::Scope iScoep(m_Isolate.get());
                    V8HandleScope hScope(m_Isolate.get());

                    for (auto it : snapData->m_FunctionTemplates)
                    {
                        V8LFuncTpl funcTemplate = isolate->GetDataFromSnapshotOnce<V8FuncTpl>(it.m_IsolateDataIndex).ToLocalChecked();
                        if (it.m_ClassName.empty() == false)
                        {
                            CppBridge::V8CppObjInfo *info = CppBridge::CallbackRegistry::GetNativeObjectInfoFromTypeName(it.m_ClassName);
                            if (info == nullptr)
                            {
                                return false;
                            }
                            SetClassFunctionTemplate(it.m_Namespace, info, funcTemplate);
                        }
                        else
                        {
                            SetFunctionTemplateInternal(it.m_FunctionName, funcTemplate, it.m_Namespace, nullptr);
                        }
                    }
                }
            }
            return true;
        }

        JSRuntimeSharedPtr JSRuntime::CloneRuntimeForSnapshotting(JSAppSharedPtr inApp)
        {
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();
            if (runtime->Initialize(inApp, m_Name, m_SnapshotIndex, m_Snapshottable, true, m_IdleEnabled) == false)
            {
                return nullptr;
            }

            // clone all of the registered functions
            std::vector<std::string> runNamespaces;
            for (auto it : m_FunctinNameMap)
            {
                std::string funcNamespace = it.second->m_Namespace;
                auto nit = std::find(runNamespaces.begin(), runNamespaces.end(), funcNamespace);
                if (nit != runNamespaces.end())
                {
                    continue;
                    ;
                }
                runNamespaces.push_back(funcNamespace);
            }
            for (auto it : runNamespaces)
            {
                V8Isolate::Scope iScope(runtime->GetIsolate());
                V8HandleScope hScope(runtime->GetIsolate());
                CppBridge::CallbackRegistry::RunNamespaceSetupFunctions(runtime, it);
            }

            for (auto it : m_Contextes)
            {
                if (it.second->SupportsSnapshots() == false)
                {
                    continue;
                }
                JSContextSharedPtr snapContext = it.second->CloneForSnapshot(runtime);
                if (snapContext == nullptr)
                {
                    return nullptr;
                }
                auto [iit, success] = runtime->m_Contextes.insert(std::make_pair(snapContext->GetName(), snapContext));
                if (success == false)
                {
                    return nullptr;
                }
            }
            return runtime;
        }

        V8TaskRunnerSharedPtr JSRuntimeIsolateHelper::GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority)
        {
            JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
            DCHECK_NOT_NULL(runtime);
            return runtime->GetForegroundTaskRunner();
        };

        bool JSRuntimeIsolateHelper::IdleTasksEnabled(V8Isolate *inIsolate)
        {
            JSRuntimeSharedPtr runtime = JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate);
            DCHECK_NOT_NULL(runtime.get());
            return runtime->IdleTasksEnabled();
        };
    } // namespace JSRuntime

    bool Serialization::TypeSerializer<v8App::JSRuntime::IdleTaskSupport>::SerializeRead(Serialization::ReadBuffer &inBuffer, v8App::JSRuntime::IdleTaskSupport &inValue)
    {
        bool enabled;
        inBuffer >> enabled;

        inValue = enabled ? v8App::JSRuntime::IdleTaskSupport::kEnabled : v8App::JSRuntime::IdleTaskSupport::kDisabled;
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::IdleTaskSupport>::SerializeWrite(Serialization::WriteBuffer &inBuffer, const v8App::JSRuntime::IdleTaskSupport &inValue)
    {
        inBuffer << (inValue == v8App::JSRuntime::IdleTaskSupport::kEnabled ? true : false);
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSRuntimeSnapshotAttributes>::SerializeRead(Serialization::ReadBuffer &inBuffer, v8App::JSRuntime::JSRuntimeSnapshotAttributes &inValue)
    {
        size_t modType;
        inBuffer >> modType;
        if (modType < 0 || modType > (int)JSRuntime::JSRuntimeSnapshotAttributes::SnapshotAndRestore)
        {
            inBuffer.SetError();
            return false;
        }
        inValue = (JSRuntime::JSRuntimeSnapshotAttributes)modType;
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSRuntimeSnapshotAttributes>::SerializeWrite(Serialization::WriteBuffer &inBuffer, const v8App::JSRuntime::JSRuntimeSnapshotAttributes &inValue)
    {
        inBuffer << (size_t)inValue;
        return inBuffer.HasErrored() == false;
    }
} // namespace v8App
