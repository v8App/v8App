// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8_PLATFORM_H_
#define _V8_PLATFORM_H_

#include <map>
#include "v8-platform.h"
#include "Threads/ThreadPool.h"
#include "JSRuntime.h"

namespace v8App
{    
    namespace JSRuntime
    {
         class V8ThreadPoolTask : public ThreadPool::ThreadPoolTask
        {
        public:
            V8ThreadPoolTask() = delete;
            explicit V8ThreadPoolTask(TaskPtr &&inTask) : m_Task(std::move(inTask))
            {
            }

            void Run() override
            {
                if (m_Task)
                {
                    m_Task->Run();
                }
            }

        private:
            std::unique_ptr<v8::Task> m_Task;
        };

       class V8Platform : public v8::Platform
        {
        public:
            V8Platform();
            ~V8Platform();

            //v8::platform interface
            v8::PageAllocator *GetPageAllocator() override;
            bool OnCriticalMemoryPressure(size_t inLength) override;

            int NumberOfWorkerThreads() override;
            std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner(v8::Isolate *inIsolate) override;
            void CallOnWorkerThread(TaskPtr inTask) override;
            void CallBlockingTaskOnWorkerThread(TaskPtr inTask) override;
            void CallLowPriorityTaskOnWorkerThread(TaskPtr inTask) override;
            void CallDelayedOnWorkerThread(TaskPtr inTask, double inDelaySeconds) override;
            bool IdleTasksEnabled(v8::Isolate *inIsolate) override;
            std::unique_ptr<v8::JobHandle> PostJob(v8::TaskPriority inPriority,
                                                   std::unique_ptr<v8::JobTask> inJobTask) override;

            //retain virtual so that testing can override it.
            virtual double MonotonicallyIncreasingTime() override;
            double CurrentClockTimeMillis() override;

            v8::TracingController* GetTracingController() override;

            void ProcessDelayedTasks();

            static void InitializeV8();
            static void ShutdownV8();

            static std::shared_ptr<V8Platform> Get();

        protected:
            std::mutex m_DelyedLock;
            std::unique_ptr<class DelayedWorkerTaskQueue> m_DelayedWorkerTasks;
            std::unique_ptr<v8::PageAllocator> m_PageAllocator;
            std::unique_ptr<v8::TracingController> m_TracingController;

            static std::shared_ptr<V8Platform> s_Platform;

            V8Platform(const V8Platform&) = delete;
            V8Platform& operator=(const V8Platform&) = delete;
        };

    } // namespace JSRuntime

} // namespace v8App
#endif
