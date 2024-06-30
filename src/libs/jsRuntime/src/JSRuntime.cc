// Copyright 2020 the v8App authors. All right reserved.
// Use of this source code is governed by the MIT license
// that can be found in the LICENSE file.

#include "v8/v8-cppgc.h"

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "Serialization/ReadBuffer.h"
#include "Utils/Format.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "JSContextModules.h"
#include "ForegroundTaskRunner.h"
#include "CppBridge/CallbackRegistry.h"
#include "JSContext.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntime::JSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName)
            : m_IdleEnabled(inEnableIdle), m_App(inApp), m_Name(inName)
        {
            m_TaskRunner = std::make_shared<ForegroundTaskRunner>();
        }

        JSRuntime::~JSRuntime()
        {
            DisposeRuntime();
        }

        JSRuntimeSharedPtr JSRuntime::CreateJSRuntime(JSAppSharedPtr inApp, IdleTasksSupport inEnableIdle, std::string inName,
                                                      JSContextCreationHelperSharedPtr inContextCreator, bool inForSnapshot)
        {
            DCHECK_NOT_NULL(inApp.get());
            JSRuntimeSharedPtr jsRuntime = std::make_shared<JSRuntime>(inApp, inEnableIdle, inName);
            jsRuntime->SetContextCreationHelper(inContextCreator);
            if (jsRuntime->CreateIsolate(inForSnapshot) == false)
            {
                return nullptr;
            }
            return jsRuntime;
        }

        void JSRuntime::Initialize()
        {
            JSContextModules::SetupModulesCallbacks(m_Isolate.get());
        }

        V8TaskRunnerSharedPtr JSRuntime::GetForegroundTaskRunner()
        {
            return m_TaskRunner;
        }

        bool JSRuntime::IdleTasksEnabled()
        {
            return m_IdleEnabled == IdleTasksSupport::kIdleTasksEnabled;
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

        void JSRuntime::SetContextCreationHelper(JSContextCreationHelperSharedPtr inCreator)
        {
            m_ContextCreation = inCreator;
        }

        JSContextCreationHelperSharedPtr JSRuntime::GetContextCreationHelper()
        {
            return m_ContextCreation;
        }

        JSContextSharedPtr JSRuntime::CreateContext(std::string inName, std::filesystem::path inEntryPoint, std::string inNamespace,
                                                    std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot, SnapshotMethod inSnapMethod)
        {
            CHECK_NOT_NULL(m_ContextCreation);
            JSContextSharedPtr context = m_ContextCreation->CreateContext(shared_from_this(), inName,
                                                                          inNamespace, inEntryPoint, inSnapEntryPoint,
                                                                          inSupportsSnapshot, inSnapMethod);
            if (context == nullptr)
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context");
                Log::Log::Error(message);
            }
            else
            {
                m_Contextes.insert(std::make_pair(inName, context));
                m_ContextCreation->RegisterSnapshotCloser(context);
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

        std::string JSRuntime::GetNamespaceForSnapIndex(size_t inSnapIndex)
        {
            DCHECK_NOT_NULL(m_ContextCreation);
            return m_ContextCreation->GetNamespaceForSnapIndex(inSnapIndex);
        }

        size_t JSRuntime::GetSnapIndexForNamespace(std::string inNamespace)
        {
            DCHECK_NOT_NULL(m_ContextCreation);
            return m_ContextCreation->GetSnapIndexForNamespace(inNamespace);
        }

        bool JSRuntime::AddSnapIndexNamespace(size_t inSnapIndex, std::string inSnapIndexName)
        {
            DCHECK_NOT_NULL(m_ContextCreation);
            return m_ContextCreation->AddSnapIndexNamespace(inSnapIndex, inSnapIndexName);
        }

        void JSRuntime::DisposeContext(JSContextSharedPtr inContext)
        {
            DCHECK_NOT_NULL(m_ContextCreation);
            if (inContext == nullptr)
            {
                return;
            }

            m_ContextCreation->UnregisterSnapshotCloser(inContext);

            m_ContextCreation->DisposeContext(inContext);
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
        }

        V8SnapshotCreatorSharedPtr JSRuntime::GetSnapshotCreator()
        {
            return m_Creator;
        }

        void JSRuntime::CloseOpenHandlesForSnapshot()
        {
            // If not a snapshotting runtime return
            if (m_Creator == nullptr)
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
            for (auto callback : m_HandleClosers)
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
            auto callback = std::find_if(m_HandleClosers.begin(), m_HandleClosers.end(),
                                         [&inCloser](const ISnapshotHandleCloserWeakPtr &inPtr)
                                         {
                                             if (inPtr.expired() == false)
                                             {
                                                 return inCloser.lock() == inPtr.lock();
                                             }
                                             return false;
                                         });
            if (callback == m_HandleClosers.end())
            {
                m_HandleClosers.push_back(inCloser);
            }
        }

        void JSRuntime::UnregisterSnapshotHandlerCloser(ISnapshotHandleCloserWeakPtr inCloser)
        {
            if (m_HandleClosers.empty())
            {
                return;
            }

            // we loop through the callbacks to find the registered callback but
            // also any callabcks that are expired just to clean them out
            bool found = true;
            while (found)
            {
                auto pos = std::find_if(m_HandleClosers.begin(), m_HandleClosers.end(),
                                        [&inCloser](const ISnapshotHandleCloserWeakPtr &inPtr)
                                        {
                                            if (inPtr.expired() == false)
                                            {
                                                return inCloser.lock() == inPtr.lock();
                                            }
                                            return true;
                                        });
                if (m_HandleClosers.end() != pos)
                {
                    m_HandleClosers.erase(pos);
                }
                else
                {
                    found = false;
                }
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

        bool JSRuntime::CreateIsolate(bool inForSnapshot)
        {
            if (m_App == nullptr || m_App->GetSnapshotProvider() == nullptr)
            {
                return false;
            }
            V8Isolate::CreateParams params;
            params.snapshot_blob = m_App->GetSnapshotProvider()->GetSnapshotData();
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

            if (inForSnapshot)
            {
                m_Creator = std::make_unique<V8SnapCreator>(isolate, params);
            }
            else
            {
                V8Isolate::Initialize(isolate, params);
            }
            return true;
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
