// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "DelayedWorkerTaskQueue.h"

namespace v8App
{
    namespace JSRuntime
    {
        class DelayedWorkerTestTask : public v8::Task
        {
            public:
                        ~DelayedWorkerTestTask(){}

            void Run() override
            {
            }
        };

        class MockDelayedWorkerTaskQueue : public DelayedWorkerTaskQueue
        {
        public:
            ~MockDelayedWorkerTaskQueue(){}
            void SetMockTime(double inTime) { m_MockTime = inTime; }
            size_t GetQueueSize() { return m_DelayedTasks.size(); }
            bool IsStopped() { return m_Stopped; }
            double MonotonicallyIncreasingTime() override { return m_MockTime; }

        protected:
            double m_MockTime = 0.0;
        };

        TEST(DelayedWorkerTaskQueueTest, PostTaskStop)
        {
            using SharedQueue = std::shared_ptr<MockDelayedWorkerTaskQueue>;
            SharedQueue queue = std::make_shared<MockDelayedWorkerTaskQueue>();

            EXPECT_EQ(0, queue->GetQueueSize());
            TaskPtr task = std::make_unique<DelayedWorkerTestTask>();
            queue->PostTask(task, 0.0);
            EXPECT_EQ(1, queue->GetQueueSize());
            EXPECT_EQ(nullptr, task.get());

            //test stop
            EXPECT_FALSE(queue->IsStopped());
            queue->Stop();
            EXPECT_EQ(0, queue->GetQueueSize());
            EXPECT_TRUE(queue->IsStopped());

            //test the lock
            queue = std::make_shared<MockDelayedWorkerTaskQueue>();
            {
                std::thread thread1([](SharedQueue queue) {
                    for (double x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<DelayedWorkerTestTask>();
                        queue->PostTask(task, x + 0.1);
                    }
                },
                                    queue);
                std::thread thread2([](SharedQueue queue) {
                    for (double x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<DelayedWorkerTestTask>();
                        queue->PostTask(task, x + 0.2);
                    }
                },
                                    queue);

                thread1.join();
                thread2.join();
            }
            EXPECT_EQ(20, queue->GetQueueSize());
        }

        TEST(DelayedWorkerTaskQueueTest, PopTask)
        {
            using SharedQueue = std::shared_ptr<MockDelayedWorkerTaskQueue>;
            SharedQueue queue = std::make_shared<MockDelayedWorkerTaskQueue>();

            //test empty queue
            TaskPtr task = queue->PopTask();
            EXPECT_EQ(nullptr, task.get());

            //now lets push 2 task in
            task = std::make_unique<DelayedWorkerTestTask>();
            //get the pointer to compare later
            v8::Task *ptr1 = task.get();
            queue->PostTask(task, 2.0);

            task = std::make_unique<DelayedWorkerTestTask>();
            //get the pointer to compare later
            v8::Task *ptr2 = task.get();
            queue->PostTask(task, 1.0);
            ASSERT_EQ(2, queue->GetQueueSize());

            //now test no time yet
            task = queue->PopTask();
            EXPECT_EQ(nullptr, task.get());

            //move the time up so both pop
            queue->SetMockTime(3.0);

            //based on the ordering we shold get task2 then task1
            task = queue->PopTask();
            EXPECT_EQ(ptr2, task.get());
            task = queue->PopTask();
            EXPECT_EQ(ptr1, task.get());

            //test the lock.
            for (double x = 0; x < 10; x++)
            {
                TaskPtr task = std::make_unique<DelayedWorkerTestTask>();
                queue->PostTask(task, 0.0);
            }
            {
                std::thread thread1([](SharedQueue queue) {
                    TaskPtr task;
                    do
                    {
                        task = queue->PopTask();
                    } while (task);
                },
                                    queue);
                std::thread thread2([](SharedQueue queue) {
                    TaskPtr task;
                    do
                    {
                        task = queue->PopTask();
                    } while (task);
                },
                                    queue);

                thread1.join();
                thread2.join();
            }
            EXPECT_EQ(0, queue->GetQueueSize());

            //test stop
            task = std::make_unique<DelayedWorkerTestTask>();
            queue->PostTask(task, 2.0);
            queue->Stop();
            task = queue->PopTask();
            EXPECT_EQ(nullptr, task.get());
        }
    } // namespace JSRuntime
} // namespace v8App