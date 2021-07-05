// Copyright 2020 the v8App authors. All right reserved.
// Use of this source code is governed by the MIT license
// that can be found in the LICENSE file.

#include "v8.h"

#include "JSRuntime.h"
#include "DelayedWorkerTaskQueue.h"
#include "TaskRunner.h"
#include "Logging/LogMacros.h"
#include "Time/Time.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntime::JSRuntime(IdleTasksSupport inEnableIdle) : m_IdleEnabled(inEnableIdle)
        {
            m_TaskRunner = std::make_shared<TaskRunner>();
            m_DelayedWorkerTasks = std::make_unique<DelayedWorkerTaskQueue>();

            //custom deleter since we have to call dispose
            m_Isolate = std::shared_ptr<v8::Isolate>(v8::Isolate::Allocate(), [](v8::Isolate *isolate) {
                isolate->Dispose();
            });
            m_Isolate->SetData(IsolateDataSlot::kJSRuntimePointer, this);
            m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true);
            
            v8::Isolate::CreateParams params;
            //TODO: replace with custom allocator
            params.array_buffer_allocator =
                v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            v8::Isolate::Initialize(m_Isolate.get(), params);
        }

        JSRuntime::~JSRuntime()
        {
            m_Isolate->SetData(IsolateDataSlot::kJSRuntimePointer, nullptr);
            m_Isolate.reset();
        }

        std::shared_ptr<v8::TaskRunner> JSRuntime::GetForegroundTaskRunner()
        {
            return m_TaskRunner;
        }

        bool JSRuntime::AreIdleTasksEnabled()
        {
            return m_IdleEnabled == IdleTasksSupport::kIdleTasksEnabled;
        }

        JSRuntime *JSRuntime::GetRuntime(v8::Isolate *inIsloate)
        {
            return static_cast<JSRuntime *>(inIsloate->GetData(IsolateDataSlot::kJSRuntimePointer));
        }

        void JSRuntime::ProcessTasks()
        {
            TaskPtr task = m_TaskRunner->PopTask();
            while (task)
            {
                {
                    v8::Locker locker(m_Isolate.get());
                    TaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run();
                }
                task = m_TaskRunner->PopTask();
            }
        }

        void JSRuntime::ProcessIdleTasks(double inTimeLeft)
        {
            if (AreIdleTasksEnabled() == false)
            {
                return;
            }

            double deadline = Time::MonotonicallyIncreasingTimeSeconds() + inTimeLeft;

            IdleTaskPtr task = m_TaskRunner->PopIdleTask();

            while (deadline > Time::MonotonicallyIncreasingTimeSeconds() && task)
            {
                {
                    v8::Locker locker(m_Isolate.get());
                    TaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run(deadline);
                }
                task = m_TaskRunner->PopIdleTask();
            }
        }

        void JSRuntime::SetObjectTemplate(void *inInfo, v8::Local<v8::ObjectTemplate> inTemplate)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            m_ObjectTemplates[inInfo] = v8::Eternal<v8::ObjectTemplate>(m_Isolate.get(), inTemplate);
        }

        v8::Local<v8::ObjectTemplate> JSRuntime::GetObjectTemplate(void *inInfo)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            ObjectTemplateMap::iterator it = m_ObjectTemplates.find(inInfo);
            if(it == m_ObjectTemplates.end())
            {
                return v8::Local<v8::ObjectTemplate>();
            }
            return it->second.Get(m_Isolate.get());
        }

        void JSRuntime::SetFunctionTemplate(void *inInfo, v8::Local<v8::FunctionTemplate> inTemplate)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            m_FunctionTemplates[inInfo] = v8::Eternal<v8::FunctionTemplate>(m_Isolate.get(), inTemplate);
        }

        v8::Local<v8::FunctionTemplate> JSRuntime::GetFunctionTemplate(void *inInfo)
        {
            CHECK_NOT_NULL(m_Isolate.get());
            FunctionTemplateMap::iterator it = m_FunctionTemplates.find(inInfo);
            if(it == m_FunctionTemplates.end())
            {
                return v8::Local<v8::FunctionTemplate>();
            }
            return it->second.Get(m_Isolate.get());
        }

        V8ExternalRegistry& JSRuntime::GetExternalRegistry()
        {
            return m_ExternalRegistry;
        }

    } // namespace JSRuntime
} // namespace v8App
