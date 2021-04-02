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
            if(AreIdleTasksEnabled() == false) {
                return;
            }

            double deadline = Time::MonotonicallyIncreasingTimeSeconds() + inTimeLeft;

            IdleTaskPtr task = m_TaskRunner->PopIdleTask();

            while(deadline > Time::MonotonicallyIncreasingTimeSeconds() && task)
            {
                {
                    v8::Locker locker(m_Isolate.get());
                    TaskRunner::TaskRunScope runScope(m_TaskRunner);
                    task->Run(deadline);
                }
                task = m_TaskRunner->PopIdleTask();
            }
        }

    } // namespace JSRuntime
} // namespace v8App
