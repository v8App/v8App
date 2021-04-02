// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include <chrono>
#include <future>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Threads/ThreadPool.h"

namespace v8App
{
    namespace ThreadPool
    {
        TEST(ThreadPoolTest, ConstructorDestructor)
        {
            //depending on the platfowm we are running on this can chance we get it.
            int hardwareThreads = std::thread::hardware_concurrency();

            //Test desctrutor's ability to actually end all the threads
            {
                ThreadPool pool = ThreadPool();
                //we also test the GetNumberOfWorkers method
                EXPECT_EQ(hardwareThreads, pool.GetNumberOfWorkers());
                //test the IsExting method
                EXPECT_FALSE(pool.IsExiting());
            }

            {
                int numThreads = std::min(2, hardwareThreads);
                ThreadPool pool = ThreadPool(numThreads);
                EXPECT_EQ(numThreads, pool.GetNumberOfWorkers());
                EXPECT_FALSE(pool.IsExiting());
            }
        }

        TEST(ThreadPoolTest, PostTask)
        {
            int task1 = 0;
            int task2 = 0;
            int task4 = 0;
            std::unique_ptr<int> task4Ptr = std::make_unique<int>(4);

            std::packaged_task<int()> task([]() { return 2; });
            std::future<int> future = task.get_future();
            ThreadPoolTaskPtr spTask = std::make_unique<CallableThreadTask>(std::move(task)); 
            auto ltask2 = [&task2]() { task2 = 1; };
            ThreadPoolTaskPtr ttask1 = std::make_unique<CallableThreadTask>([&task1]() { task1 = 1; }); 
            ThreadPoolTaskPtr ttask2 = std::make_unique<CallableThreadTask>(ltask2); 
            ThreadPoolTaskPtr ttask4 = std::make_unique<CallableThreadTask>([task = std::move(task4Ptr), &task4](){ task4 = (*task); }); 

            {
                //Force one thread so we know the worker is able to pull work.
                ThreadPool pool = ThreadPool(1);
                bool result = pool.PostTask(std::move(ttask1));
                EXPECT_TRUE(result);
                 result = pool.PostTask(std::move(ttask2));
                EXPECT_TRUE(result);
                //test passing a  std::packaged_task<>
                result = pool.PostTask(std::move(spTask));
                EXPECT_TRUE(result);
                result = pool.PostTask(std::move(ttask4));
                EXPECT_TRUE(result);
                //sleep for a few seconds to allow the threads to work
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }

            EXPECT_EQ(1, task1);
            EXPECT_EQ(1, task2);
            EXPECT_EQ(2, future.get());
            EXPECT_EQ(4, task4);
        }

        TEST(ThreadPoolTest, Singleton)
        {
            WeakThreadPoolPtr pool = ThreadPool::Get();

            EXPECT_TRUE(pool.expired());

            pool = ThreadPool::CreateSingleton(1);

            {
                SharedThreadPoolPtr ptr = pool.lock();
                ASSERT_TRUE(ptr != nullptr);
                EXPECT_EQ(1, ptr->GetNumberOfWorkers());

                //shut it down while holding the ptr should prevent it from being destroyed
                //so we should see the it marked as exiting
                ThreadPool::ShutdownSingleton();

                EXPECT_TRUE(ptr->IsExiting());
            }

            //shared ptr went away which should have freed up the object
            //now chek we are back to a nullptr
            pool = ThreadPool::Get();

            EXPECT_TRUE(pool.lock() == nullptr);
        }
    } // namespace ThreadPool
} // namespace v8App