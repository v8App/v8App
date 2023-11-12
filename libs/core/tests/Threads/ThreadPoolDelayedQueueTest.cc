// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include <chrono>
#include <future>
#include <iostream>
#include <iomanip>

#if defined(V8APP_WINDOWS)
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Threads/ThreadPoolDelayedQueue.h"

namespace v8App
{
    namespace Threads
    {
        class TestThreadPoolDelayedTask : public IThreadPoolTask
        {
        public:
            TestThreadPoolDelayedTask(int *inValuePtr) : valuePtr(inValuePtr) {}
            int *valuePtr;
            void Run() override
            {
                if (valuePtr != nullptr)
                    *valuePtr = 5;
            }
        };

        class TestThreadPoolDelayedQueue : public ThreadPoolDelayedQueue
        {
        public:
            TestThreadPoolDelayedQueue(int inNumberOfWorkers = -1, ThreadPriority inPriority = ThreadPriority::kDefault)
                : ThreadPoolDelayedQueue(inNumberOfWorkers, inPriority) {}
            int GetThreadPriority(int inIndex)
            {
                if (inIndex >= m_Workers.size())
                {
                    return -9999;
                }
                return Threads::GetThreadPriority(m_Workers[inIndex].get());
            }
            size_t GetWorkersSize() { return m_Workers.size(); }
            bool HasTask() { return m_Queue.MayHaveItems(); }
        };

        TEST(ThreadPoolDelayedQueueTest, Constructor)
        {
            // depending on the platfowm we are running on this can chance we get it.
            int hardwareThreads = std::thread::hardware_concurrency() - 1;

            // Test desctrutor's ability to actually end all the threads
            {
                TestThreadPoolDelayedQueue pool = TestThreadPoolDelayedQueue();
                // we also test the GetNumberOfWorkers method
                EXPECT_EQ(hardwareThreads, pool.GetNumberOfWorkers());
                // test the IsExting method
                EXPECT_FALSE(pool.IsExiting());
                EXPECT_EQ(hardwareThreads, pool.GetWorkersSize());

#if defined(V8APP_WINDOWS)
                EXPECT_EQ(THREAD_PRIORITY_NORMAL, pool.GetThreadPriority(0));
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
                EXPECT_EQ(5, pool.GetThreadPriority(0));
#endif
            }

            {
                int numThreads = std::min(2, hardwareThreads);
                TestThreadPoolDelayedQueue pool = TestThreadPoolDelayedQueue(numThreads, ThreadPriority::kUserBlocking);
                EXPECT_EQ(numThreads, pool.GetNumberOfWorkers());
#if defined(V8APP_WINDOWS)
                EXPECT_EQ(THREAD_PRIORITY_TIME_CRITICAL, pool.GetThreadPriority(0));
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
                EXPECT_EQ(10, pool.GetThreadPriority(0));
#endif
            }
            {
                TestThreadPoolDelayedQueue pool = TestThreadPoolDelayedQueue(0);
                EXPECT_EQ(1, pool.GetNumberOfWorkers());
            }
        }

        TEST(ThreadPoolDelayedQueueTest, PostTask)
        {
            int task1 = 0;
            int task2 = 0;
            int task4 = 0;
            std::unique_ptr<int> task4Ptr = std::make_unique<int>(4);
            int task5 = 0;

            //Make sure the test time function is disabled for this test
            TestTime::TestTimeSeconds::Clear();
            std::packaged_task<int()> task([]()
                                           { return 2; });
            std::future<int> future = task.get_future();
            ThreadPoolTaskUniquePtr spTask = std::make_unique<CallableThreadTask>(std::move(task));
            auto ltask2 = [&task2]()
            { task2 = 1; };
            ThreadPoolTaskUniquePtr ttask1 = std::make_unique<CallableThreadTask>([&task1]()
                                                                                  { task1 = 1; });
            ThreadPoolTaskUniquePtr ttask2 = std::make_unique<CallableThreadTask>(ltask2);
            ThreadPoolTaskUniquePtr ttask4 = std::make_unique<CallableThreadTask>([task = std::move(task4Ptr), &task4]()
                                                                                  { task4 = (*task); });

            ThreadPoolTaskUniquePtr ttask5 = std::make_unique<TestThreadPoolDelayedTask>(&task5);

            {
                // Force one thread so we know the worker is able to pull work.
                TestThreadPoolDelayedQueue pool = TestThreadPoolDelayedQueue(1);
                bool result = pool.PostTask(std::move(ttask1));
                EXPECT_TRUE(result);
                result = pool.PostTask(std::move(ttask2));
                EXPECT_TRUE(result);
                // test passing a  std::packaged_task<>
                result = pool.PostTask(std::move(spTask));
                EXPECT_TRUE(result);
                result = pool.PostDelayedTask(5.0, std::move(ttask4));
                EXPECT_TRUE(result);
                result = pool.PostDelayedTask(5.0, std::move(ttask5));
                EXPECT_TRUE(result);
                // sleep for a few seconds to allow the threads to work
                std::this_thread::sleep_for(std::chrono::seconds(2));
                EXPECT_EQ(1, task1);
                EXPECT_EQ(1, task2);
                EXPECT_EQ(2, future.get());
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }

            EXPECT_EQ(4, task4);
            EXPECT_EQ(5, task5);
        }

        TEST(ThreadPoolDelayedQueueTest, Terminates)
        {
            TestThreadPoolDelayedQueue pool = TestThreadPoolDelayedQueue(1);
            ThreadPoolTaskUniquePtr ttask1 = std::make_unique<CallableThreadTask>([]() {});

            pool.Terminate();
            EXPECT_TRUE(pool.IsExiting());
            pool.PostDelayedTask(1.0, std::move(ttask1));
            EXPECT_FALSE(pool.HasTask());
        }
    } // namespace Threads
} // namespace v8App