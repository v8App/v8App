// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <future>
#include <functional>
#include <chrono>
#include "v8.h"
#include "V8Platform.h"
#include "Time/Time.h"
#include "Logging/LogMacros.h"
#include "DelayedWorkerTaskQueue.h"

namespace v8App
{
    namespace JSRuntime
    {
        std::shared_ptr<V8Platform> V8Platform::s_Platform;

        V8Platform::V8Platform()
        {
            m_DelayedWorkerTasks = std::make_unique<DelayedWorkerTaskQueue>();
            m_TracingController = std::make_unique<v8::TracingController>();
        }

        V8Platform::~V8Platform()
        {
        }

        //v8::platform interface
        v8::PageAllocator *V8Platform::GetPageAllocator()
        {
            //for now just returns a nullptr
            return m_PageAllocator.get();
        }

        bool V8Platform::OnCriticalMemoryPressure(size_t inLength)
        {
            //for now we return false
            return false;
        }

        int V8Platform::NumberOfWorkerThreads()
        {
            //make sure it's not null in debug
            ThreadPool::WeakThreadPoolPtr pool = ThreadPool::ThreadPool::Get();
            DCHECK_EQ(pool.expired(), false);
            //return the number of worker we have in the global thread pool
            return pool.lock()->GetNumberOfWorkers();
        }

        std::shared_ptr<v8::TaskRunner> V8Platform::GetForegroundTaskRunner(v8::Isolate *inIsolate)
        {
            //make sure it's not null in debug
            DCHECK_NE(JSRuntime::GetRuntime(inIsolate), nullptr);
            return JSRuntime::GetRuntime(inIsolate)->GetForegroundTaskRunner();
        }

        void V8Platform::CallOnWorkerThread(TaskPtr inTask)
        {
            //make sure it's not null in debug
            ThreadPool::WeakThreadPoolPtr pool = ThreadPool::ThreadPool::Get();
            DCHECK_EQ(pool.expired(), false);
             ThreadPool::ThreadPoolTaskPtr task = std::make_unique<V8ThreadPoolTask>(std::move(inTask));
            pool.lock()->PostTask(std::move(task));
        }

        void V8Platform::CallBlockingTaskOnWorkerThread(TaskPtr inTask)
        {
            //make sure it's not null in debug
            ThreadPool::WeakThreadPoolPtr pool = ThreadPool::ThreadPool::Get();
            DCHECK_EQ(pool.expired(), false);
             std::packaged_task<void()> packed([task = std::move(inTask)]() { task->Run(); });
            //get the future
            std::future<void> future = packed.get_future();
            ThreadPool::ThreadPoolTaskPtr task = std::make_unique<ThreadPool::CallableThreadTask>(std::move(packed));
            pool.lock()->PostTask(std::move(task));
            //block till it's done
            future.get();
        }

        void V8Platform::CallLowPriorityTaskOnWorkerThread(TaskPtr inTask)
        {
            //make sure it's not null in debug
             ThreadPool::WeakThreadPoolPtr pool = ThreadPool::ThreadPool::Get();
            DCHECK_EQ(pool.expired(), false);
            //TODO: Add priorty to the threak pool
            ThreadPool::ThreadPoolTaskPtr task = std::make_unique<V8ThreadPoolTask>(std::move(inTask));
            pool.lock()->PostTask(std::move(task));
        }

        void V8Platform::CallDelayedOnWorkerThread(TaskPtr inTask, double inDelaySeconds)
        {
            m_DelayedWorkerTasks->PostTask(inTask, inDelaySeconds);
        }

        bool V8Platform::IdleTasksEnabled(v8::Isolate *inIsolate)
        {
            //make sure it's not null in debug
            DCHECK_NE(JSRuntime::GetRuntime(inIsolate), nullptr);
            return JSRuntime::GetRuntime(inIsolate)->AreIdleTasksEnabled();
        }

        std::unique_ptr<v8::JobHandle> V8Platform::PostJob(
            v8::TaskPriority inPriority, std::unique_ptr<v8::JobTask> inJobTask)
        {
            //for now just return the default. Looking through chromium's source don't see it used.
            return {};
        }

        double V8Platform::MonotonicallyIncreasingTime()
        {
            return Time::MonotonicallyIncreasingTimeSeconds();
        }

        double V8Platform::CurrentClockTimeMillis()
        {
            return std::chrono::duration<double, std::milli>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

        v8::TracingController *V8Platform::GetTracingController()
        {
            return m_TracingController.get();
        }

        void V8Platform::ProcessDelayedTasks()
        {
            TaskPtr task;
            do
            {
                task = m_DelayedWorkerTasks->PopTask();
                if (task)
                {
                    ThreadPool::ThreadPoolTaskPtr ptask = std::make_unique<V8ThreadPoolTask>(std::move(task));
                    ThreadPool::ThreadPool::Get().lock()->PostTask(std::move(ptask));
                    continue;
                }
            } while (task);
        }

        void V8Platform::InitializeV8()
        {
            if (s_Platform != nullptr)
            {
                return;
            }

            s_Platform = std::make_shared<V8Platform>();
            v8::V8::InitializePlatform(s_Platform.get());
            v8::V8::Initialize();
        }

        void V8Platform::ShutdownV8()
        {
            v8::V8::ShutdownPlatform();
            s_Platform.reset();
        }

        std::shared_ptr<V8Platform> V8Platform::Get()
        {
            return s_Platform;
        }

    } // namespace JSRuntime
} // namespace v8App