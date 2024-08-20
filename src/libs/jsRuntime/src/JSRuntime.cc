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

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntime::JSRuntime(JSAppSharedPtr inApp, IdleTaskSupport inEnableIdle,
                             std::string inName, bool inSetupForSnapshot, size_t inRuntimeIndex)
            : m_IdleEnabled(inEnableIdle), m_App(inApp), m_Name(inName),
              m_IsSnapshotter(inSetupForSnapshot), m_SnapshotIndex(inRuntimeIndex)
        {
            DCHECK_NOT_NULL(inApp.get());
            m_TaskRunner = std::make_shared<ForegroundTaskRunner>();
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
            m_CppHeapID = inRuntime.m_CppHeapID;

            m_Initialized = inRuntime.m_Initialized;
            inRuntime.m_Initialized = false;
        }

        bool JSRuntime::Initialize(bool isSnapshottable)
        {
            if (m_Initialized)
            {
                return true;
            }

            if (CreateIsolate() == false)
            {
                return false;
            }

            JSContextModules::SetupModulesCallbacks(m_Isolate.get());
            m_Initialized = true;
            m_Snapshottable = isSnapshottable;
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

        void JSRuntime::SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            m_ObjectTemplates[inInfo] = v8::Global<v8::ObjectTemplate>(m_Isolate.get(), inTemplate);
        }

        v8::Local<v8::ObjectTemplate> JSRuntime::GetObjectTemplate(void *inInfo)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            ObjectTemplateMap::iterator it = m_ObjectTemplates.find(inInfo);
            if (it == m_ObjectTemplates.end())
            {
                return v8::Local<v8::ObjectTemplate>();
            }
            return it->second.Get(m_Isolate.get());
        }

        JSContextSharedPtr JSRuntime::CreateContext(std::string inName, std::filesystem::path inEntryPoint, std::string inNamespace,
                                                    std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot, SnapshotMethod inSnapMethod)
        {
            if (GetContextProvider() == nullptr)
            {
                return nullptr;
            }
            size_t contextIndex = m_ContextNamespaces.GetIndexForName(ResolveContextName(inName, inNamespace, inSnapMethod));
            if (contextIndex == m_ContextNamespaces.GetMaxSupportedIndexes())
            {
                contextIndex = 0;
            }
            JSContextSharedPtr context = GetContextProvider()->CreateContext(shared_from_this(), inName,
                                                                             inNamespace, inEntryPoint, inSnapEntryPoint,
                                                                             inSupportsSnapshot, inSnapMethod, contextIndex);
            if (context == nullptr)
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context");
                Log::Log::Error(message);
            }
            else
            {
                m_Contextes.insert(std::make_pair(inName, context));
                // m_ContextProvider->RegisterSnapshotCloser(context);
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
            m_Creator.reset();
            m_HandleClosers.clear();
            if (m_Isolate != nullptr)
            {
                V8IsolateScope isolateScope(m_Isolate.get());
                V8Locker locker(m_Isolate.get());
                V8HandleScope handleScope(m_Isolate.get());
                for (auto &it : m_ObjectTemplates)
                {
                    it.second.Reset();
                }
                m_Contextes.clear();
                JSRuntimeWeakPtr *weakPtr = static_cast<JSRuntimeWeakPtr *>(m_Isolate->GetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr)));
                delete weakPtr;
                m_Isolate->SetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr), nullptr);
            }
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
            V8LContext defaultContext = V8Context::New(m_Isolate.get());

            // we always add a v8 context to the snapshot so nothing is ever assigned
            m_Creator->SetDefaultContext(defaultContext);

            Containers::NamedIndexes contextIndexes;
            if (contextIndexes.AddNamedIndex(0, "v8-default") == false)
            {
                return false;
            }

            for (auto [name, context] : m_Contextes)
            {
                if (context->SupportsSnapshots() == false)
                {
                    continue;
                }
                V8Isolate::Scope(m_Isolate.get());
                V8HandleScope hScope(m_Isolate.get());

                size_t index = m_Creator->AddContext(context->GetLocalContext());
                if (contextIndexes.AddNamedIndex(index, name) == false)
                {
                    LOG_ERROR(Utils::format("Failed to add the context {} to the named indexes", name));
                    return false;
                }
            }
            CloseOpenHandlesForSnapshot();
            V8StartupData snapshot = m_Creator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);
            if (snapshot.data == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to create the snapshot for runtime {}", m_Name));
                return false;
            }
            inBuffer << m_Name;
            if (contextIndexes.SerializeNameIndexes(inBuffer) == false)
            {
                LOG_ERROR(Utils::format("Failed to seriaize the context indexes for runtime {}", m_Name));
                return false;
            }
            inBuffer.SerializeWrite(snapshot.data, snapshot.raw_size);
            return true;
        }

        bool JSRuntime::RestoreSnapshot(Serialization::ReadBuffer &inBufffer, void *inData)
        {
            return false;
        }

        void JSRuntime::CloseOpenHandlesForSnapshot()
        {
            // If not a snapshotting runtime return
            if (m_IsSnapshotter == false)
            {
                return;
            }
            if (m_ObjectTemplates.empty() == false)
            {

                for (auto it = m_ObjectTemplates.begin(); it != m_ObjectTemplates.end(); it++)
                {
                    it->second.Reset();
                }
            }

            m_ObjectTemplates.clear();
            for (auto [closerPtr, callback] : m_HandleClosers)
            {
                if (callback.expired())
                {
                    continue;
                }
                callback.lock()->CloseHandleForSnapshot();
            }
            // clear the callbacks since they are all called
            m_HandleClosers.clear();
        }

        void JSRuntime::RegisterSnapshotHandleCloser(ISnapshotHandleCloserWeakPtr inCloser)
        {
            //not a snapshotter no reason to register them
            if(m_IsSnapshotter == false)
            {
                return;
            }
            //no point in adding an expired ptr
            if(inCloser.expired())
            {
                return;
            }
            ISnapshotHandleCloser * closerPtr = inCloser.lock().get();
            if(m_HandleClosers.find(closerPtr) != m_HandleClosers.end())
            {
                return;
            }
            m_HandleClosers.emplace(closerPtr, inCloser);
        }

        void JSRuntime::UnregisterSnapshotHandlerCloser(ISnapshotHandleCloser *inCloser)
        {
            //not a snapshotter no reason to unregister them
            if(m_IsSnapshotter == false)
            {
                return;
            }

            //if empty no need to go on
            if (m_HandleClosers.empty())
            {
                return;
            }

            // we loop through the callbacks to find the registered callback but
            // also any callabcks that are expired just to clean them out
            bool found = true;
            std::vector<ISnapshotHandleCloser*> removePos;
            for(auto [closerPtr, closerWeakPtr]: m_HandleClosers)
            {
                if (closerPtr == inCloser)
                {
                    removePos.push_back(closerPtr);
                }
                else if (closerWeakPtr.expired())
                {
                    removePos.push_back(closerPtr);
                }                
            }
            //now acutaly remove them
            for(auto closerPtr: removePos)
            {
                auto pos = m_HandleClosers.find(closerPtr);
                m_HandleClosers.erase(pos);
            }
        }

        V8CppHeap *JSRuntime::GetCppHeap()
        {
            if (m_Isolate == nullptr)
            {
                return nullptr;
            }
            return m_Isolate->GetCppHeap();
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
            params.external_references = m_App->GetSnapshotProvider()->GetExternalReferences();

            // custom deleter since we have to call dispose
            V8Isolate *isolate = V8Isolate::Allocate();
            m_Isolate = std::shared_ptr<V8Isolate>(isolate, [](V8Isolate *isolate)
                                                   { isolate->Dispose(); });
            m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true);

            JSRuntimeWeakPtr *weakPtr = new JSRuntimeWeakPtr(shared_from_this());
            m_Isolate->SetData(uint32_t(JSRuntime::DataSlot::kJSRuntimeWeakPtr), weakPtr);

            // TODO: replace with custom allocator
            params.array_buffer_allocator =
                V8ArrayBuffer::Allocator::NewDefaultAllocator();

            V8CppHeapUniquePtr heap = V8CppHeap::Create(
                V8AppPlatform::Get().get(),
                v8::CppHeapCreateParams({}, v8::WrapperDescriptor((int)V8CppObjDataIntField::ObjInfo, (int)V8CppObjDataIntField::ObjInstance, m_CppHeapID)));
            params.cpp_heap = heap.get();
            // the isolate will own the heap so release it
            heap.release();

            if (m_IsSnapshotter)
            {
                m_Creator = std::make_unique<V8SnapCreator>(isolate, params);
            }
            else
            {
                V8Isolate::Initialize(isolate, params);
            }
            return true;
        }

        JSRuntimeSharedPtr JSRuntime::CloneRuntimeForSnapshotting(JSAppSharedPtr inApp)
        {
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(inApp, m_IdleEnabled, m_Name, true, m_SnapshotIndex);
            if (runtime->Initialize(true) == false)
            {
                return nullptr;
            }
            for (auto it : m_Contextes)
            {
                if (it.second->SupportsSnapshots() == false)
                {
                    continue;
                }
                JSContextSharedPtr snapContext = it.second->CloneForSnapshot(shared_from_this());
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

        bool JSRuntime::AddContextesToSnapshot()
        {
            if (m_Creator == nullptr)
            {
                return false;
            }
            IJSSnapshotCreatorSharedPtr snapCreator = m_App->GetSnapshotCreator();
            for (auto it : m_Contextes)
            {
                size_t index = m_Creator->AddContext(it.second->GetLocalContext(), snapCreator->GetInternalSerializerCallaback(),
                                                     snapCreator->GetContextSerializerCallback());
                m_ContextNamespaces.AddNamedIndex(index, it.second->GetNamespace());
            }
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
} // namespace v8App
