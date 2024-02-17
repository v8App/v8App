// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Queues/TThreadSafeDelayedQueue.h"
#include "TestTime.h"

namespace v8App
{
    namespace Queues
    {
        static float delayedTime = 5.0;
        double TestTime()
        {
            return delayedTime;
        }

        class TestDelayedQueueTask
        {
        public:
            ~TestDelayedQueueTask() {}

            void Run()
            {
            }
        };

        static bool notifier = false;
        void TestNotifier()
        {
            notifier = true;
        }

        using TaskDelayedTaskUniquePtr = std::unique_ptr<TestDelayedQueueTask>;

        class TestDelayedTaskQueue : public TThreadSafeDelayedQueue<TaskDelayedTaskUniquePtr>
        {
        public:
            TestDelayedTaskQueue() : TThreadSafeDelayedQueue() {}
            size_t GetQueueSize() { return m_Queue.size(); }
            size_t GetDelayedQueueSize() { return m_DelayedQueue.size(); }
            bool IsTerminated() { return m_Terminated; }
        };

        TEST(TThreadSafeDelayedQueue, Constrcutor)
        {
            TestDelayedTaskQueue noCustomTime = TestDelayedTaskQueue();
            TestDelayedTaskQueue customTime = TestDelayedTaskQueue();

            EXPECT_EQ(false, noCustomTime.IsTerminated());
            EXPECT_EQ(0, noCustomTime.GetQueueSize());
            EXPECT_EQ(0, noCustomTime.GetDelayedQueueSize());
        }

        TEST(TThreadSafeDelayedQueue, PushItemGetNextItem)
        {
            TestDelayedTaskQueue queue = TestDelayedTaskQueue();
            queue.SetDelayedJobsReadyDelegate(&TestNotifier);
            TaskDelayedTaskUniquePtr taskPtr1 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr2 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr3 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr4 = std::make_unique<TestDelayedQueueTask>();
            TestDelayedQueueTask *task1 = taskPtr1.get();
            TestDelayedQueueTask *task2 = taskPtr2.get();
            TestDelayedQueueTask *task3 = taskPtr3.get();

            // enable the test time function
            TestTime::TestTimeSeconds::Enable();
            TestTime::TestTimeSeconds::Set(0);
            EXPECT_FALSE(queue.GetNextItem());
            queue.PushItem(std::move(taskPtr1));
            EXPECT_EQ(1, queue.GetQueueSize());

            queue.PushItemDelayed(4.0, std::move(taskPtr2));
            EXPECT_EQ(1, queue.GetDelayedQueueSize());
            queue.PushItemDelayed(4.0, std::move(taskPtr3));
            EXPECT_EQ(2, queue.GetDelayedQueueSize());
            queue.PushItemDelayed(6.0, std::move(taskPtr4));
            EXPECT_EQ(3, queue.GetDelayedQueueSize());

            auto opt = queue.GetNextItem();
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task1);
            EXPECT_EQ(0, queue.GetQueueSize());
            EXPECT_EQ(3, queue.GetDelayedQueueSize());

            TestTime::TestTimeSeconds::Set(5);
            opt = queue.GetNextItem();
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task2);
            EXPECT_EQ(1, queue.GetQueueSize());
            EXPECT_EQ(1, queue.GetDelayedQueueSize());
            EXPECT_TRUE(notifier);

            opt =queue.GetNextItem();
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task3);
            EXPECT_EQ(0, queue.GetQueueSize());
            EXPECT_EQ(1, queue.GetDelayedQueueSize());

            EXPECT_FALSE(queue.GetNextItem());
        }

        TEST(TThreadSafeDelayedQueue, MayHaveItems)
        {
            TestDelayedTaskQueue queue = TestDelayedTaskQueue();
            TaskDelayedTaskUniquePtr taskPtr1 = std::make_unique<TestDelayedQueueTask>();

            TestTime::TestTimeSeconds::Enable();
            TestTime::TestTimeSeconds::Set(4);
            EXPECT_FALSE(queue.MayHaveItems());
            queue.PushItemDelayed(1.0, std::move(taskPtr1));
            EXPECT_FALSE(queue.MayHaveItems());
            TestTime::TestTimeSeconds::Set(6);
            EXPECT_TRUE(queue.MayHaveItems());
        }

        TEST(TThreadSafeDelayedQueue, Terminates)
        {
            TestDelayedTaskQueue queue = TestDelayedTaskQueue();
            TaskDelayedTaskUniquePtr taskPtr1 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr2 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr3 = std::make_unique<TestDelayedQueueTask>();

            queue.PushItem(std::move(taskPtr1));
            EXPECT_EQ(1, queue.GetQueueSize());

            queue.PushItemDelayed(4.0, std::move(taskPtr2));
            EXPECT_EQ(1, queue.GetDelayedQueueSize());

            queue.Terminate();
            EXPECT_TRUE(queue.IsTerminated());
            EXPECT_FALSE(queue.GetNextItem());
            EXPECT_EQ(1, queue.GetDelayedQueueSize());
            EXPECT_EQ(1, queue.GetQueueSize());
            queue.PushItemDelayed(4.0, std::move(taskPtr3));
            EXPECT_EQ(1, queue.GetQueueSize());
        }
    } // namespace Queues
} // namespace v8App