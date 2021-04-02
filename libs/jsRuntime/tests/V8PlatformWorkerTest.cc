// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include <future>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "V8Platform.h"

namespace v8App
{
    namespace JSRuntime
    {
        class V8PlatformTestTask : public v8::Task
        {
            public:
            V8PlatformTestTask(std::shared_ptr<int> inInt) : m_Int(inInt)
            {
            }

            void Run() override
            {
                (*m_Int) = 5;
            }

        protected:
            std::shared_ptr<int> m_Int;
        };

        class V8PlatformWorkerTestTask : public v8::Task
        {
            public:
            V8PlatformWorkerTestTask(std::shared_ptr<std::packaged_task<void()>> inFuture) : m_Future(inFuture)
            {
            }

            void Run() override
            {
                (*m_Future)();
            }

        protected:
            std::shared_ptr<std::packaged_task<void()>> m_Future;
        };

        TEST(V8PlatformTest, V8ThreadPoolTask)
        {
            std::shared_ptr<int> sharedInt = std::make_shared<int>(0);
            TaskPtr v8Task = std::make_unique<V8PlatformTestTask>(sharedInt);
            V8ThreadPoolTask task(std::move(v8Task));
            EXPECT_EQ(nullptr, v8Task.get());
            task.Run();
            EXPECT_EQ(5, *sharedInt);
        }

        TEST(V8PlatformTest, GetPageAllocator)
        {
            V8Platform platform;
            EXPECT_EQ(nullptr, platform.GetPageAllocator());
        }

        TEST(V8PlatformTest, OnCriticalMemoryPressure)
        {
            V8Platform platform;
            EXPECT_FALSE(platform.OnCriticalMemoryPressure(100));
        }

        TEST(V8PlatformTest, NumberOfWorkerThreads)
        {
            //safe to call multiple times as if the singleton is created it skips and just returns it.
            ThreadPool::ThreadPool::CreateSingleton(std::thread::hardware_concurrency());

            V8Platform platform;
            EXPECT_EQ(std::thread::hardware_concurrency(), platform.NumberOfWorkerThreads());
        }

        TEST(V8PlatformTest, CallOnWorkerThread)
        {
            //safe to call multiple times as if the singleton is created it skips and just returns it.
            ThreadPool::ThreadPool::CreateSingleton(std::thread::hardware_concurrency());

            V8Platform platform;
            int test1 = 0;
            std::shared_ptr<std::packaged_task<void()>> pTask = std::make_shared<std::packaged_task<void()>>([&test1]() { test1 = 5; });
            TaskPtr v8Task = std::make_unique<V8PlatformWorkerTestTask>(pTask);
            std::future<void> future = pTask->get_future();
            platform.CallOnWorkerThread(std::move(v8Task));
            future.get();
            EXPECT_EQ(5, test1);
        }

        TEST(V8PlatformTest, CallBlockingOnWorkerThread)
        {
            //safe to call multiple times as if the singleton is created it skips and just returns it.
            ThreadPool::ThreadPool::CreateSingleton(std::thread::hardware_concurrency());

            V8Platform platform;
            std::shared_ptr<int> sharedInt = std::make_shared<int>(0);
            TaskPtr v8Task = std::make_unique<V8PlatformTestTask>(sharedInt);
            platform.CallBlockingTaskOnWorkerThread(std::move(v8Task));
            EXPECT_EQ(5, *sharedInt);
        }

        TEST(V8PlatformTest, CallLowPriorityTaskOnWorkerThread)
        {
            //safe to call multiple times as if the singleton is created it skips and just returns it.
            ThreadPool::ThreadPool::CreateSingleton(std::thread::hardware_concurrency());

            V8Platform platform;
            int test1 = 0;
            std::shared_ptr<std::packaged_task<void()>> pTask = std::make_shared<std::packaged_task<void()>>([&test1]() { test1 = 5; });
            TaskPtr v8Task = std::make_unique<V8PlatformWorkerTestTask>(pTask);
            std::future<void> future = pTask->get_future();
            platform.CallLowPriorityTaskOnWorkerThread(std::move(v8Task));
            future.get();
            EXPECT_EQ(5, test1);
        }

        TEST(V8PlatformTest, CallDelayedOnWorkerThreadProcessDelayedTasks)
        {
            //safe to call multiple times as if the singleton is created it skips and just returns it.
            ThreadPool::ThreadPool::CreateSingleton(std::thread::hardware_concurrency());

            V8Platform platform;
            int test1 = 0;
            std::shared_ptr<std::packaged_task<void()>> pTask = std::make_shared<std::packaged_task<void()>>([&test1]() { test1 = 5; });
            TaskPtr v8Task = std::make_unique<V8PlatformWorkerTestTask>(pTask);
            std::future<void> future = pTask->get_future();
            platform.CallDelayedOnWorkerThread(std::move(v8Task), 0.0);
            platform.ProcessDelayedTasks();
            future.get();
            EXPECT_EQ(5, test1);
        }

        TEST(V8PlatformTest, PostJob)
        {
            V8Platform platform;
            std::unique_ptr<v8::JobTask> task;
            EXPECT_EQ(nullptr, platform.PostJob(v8::TaskPriority::kBestEffort, std::move(task)).get());

        }
    } // namespace JSRuntime
} // namespace v8App