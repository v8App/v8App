// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "NestableQueue.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestDelayedQueueTask : public v8::Task
        {
        public:
            ~TestDelayedQueueTask() {}

            virtual void Run() override
            {
            }
        };

        using TaskDelayedTaskUniquePtr = std::unique_ptr<TestDelayedQueueTask>;

        class TestDelayedTaskQueue : public NestableQueue
        {
        public:
            TestDelayedTaskQueue() : NestableQueue() {}
            size_t GetQueueSize() { return m_Queue.size(); }
            bool IsTerminated() { return m_Terminated; }
        };

        TEST(TThreadSafeDelayedQueue, Constrcutor)
        {
            TestDelayedTaskQueue noCustomTime = TestDelayedTaskQueue();
            TestDelayedTaskQueue customTime = TestDelayedTaskQueue();

            EXPECT_EQ(0, noCustomTime.GetQueueSize());
        }

        TEST(TThreadSafeDelayedQueue, PushItemGetNextItem0Depth)
        {
            TestDelayedTaskQueue queue = TestDelayedTaskQueue();
            TaskDelayedTaskUniquePtr taskPtr1 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr2 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr3 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr4 = std::make_unique<TestDelayedQueueTask>();
            TestDelayedQueueTask *task1 = taskPtr1.get();
            TestDelayedQueueTask *task2 = taskPtr2.get();
            TestDelayedQueueTask *task3 = taskPtr3.get();
            TestDelayedQueueTask *task4 = taskPtr4.get();

            TestTime::TestTimeSeconds::Enable();
            TestTime::TestTimeSeconds::Set(0);

            EXPECT_FALSE(queue.GetNextItem(0));
            queue.PushItem(std::move(taskPtr1));
            EXPECT_EQ(1, queue.GetQueueSize());

            queue.PushNonNestableItemDelayed(4.0, std::move(taskPtr2));
            queue.PushItemDelayed(4.0, std::move(taskPtr3));
            queue.PushNonNestableItemDelayed(6.0, std::move(taskPtr4));

            auto opt = queue.GetNextItem(0);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task1);
            EXPECT_EQ(0, queue.GetQueueSize());

            TestTime::TestTimeSeconds::Set(5);

            opt = queue.GetNextItem(0);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task2);
            EXPECT_EQ(1, queue.GetQueueSize());

            opt = queue.GetNextItem(0);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task3);
            EXPECT_EQ(0, queue.GetQueueSize());

            TestTime::TestTimeSeconds::Set(7);
            opt = queue.GetNextItem(0);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task4);
            EXPECT_EQ(0, queue.GetQueueSize());

            EXPECT_FALSE(queue.GetNextItem(0));
        }

        TEST(TThreadSafeDelayedQueue, PushItemGetNextItem2Depth)
        {
            TestDelayedTaskQueue queue = TestDelayedTaskQueue();
            TaskDelayedTaskUniquePtr taskPtr1 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr2 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr3 = std::make_unique<TestDelayedQueueTask>();
            TaskDelayedTaskUniquePtr taskPtr4 = std::make_unique<TestDelayedQueueTask>();
            TestDelayedQueueTask *task1 = taskPtr1.get();
            TestDelayedQueueTask *task2 = taskPtr2.get();
            TestDelayedQueueTask *task3 = taskPtr3.get();
            TestDelayedQueueTask *task4 = taskPtr4.get();

            EXPECT_FALSE(queue.GetNextItem(2));
            queue.PushNonNestableItem(std::move(taskPtr1));
            queue.PushItem(std::move(taskPtr2));
            queue.PushNonNestableItem(std::move(taskPtr3));
            queue.PushItem(std::move(taskPtr4));
            EXPECT_EQ(4, queue.GetQueueSize());

            auto opt = queue.GetNextItem(2);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task2);
            EXPECT_EQ(3, queue.GetQueueSize());

            opt = queue.GetNextItem(2);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task4);
            EXPECT_EQ(2, queue.GetQueueSize());

            opt = queue.GetNextItem(2);
            EXPECT_FALSE(opt.has_value());
            EXPECT_EQ(2, queue.GetQueueSize());

            // set nesting to 0 and we should get our 2 non nestable tasks
            opt = queue.GetNextItem(0);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task1);
            EXPECT_EQ(1, queue.GetQueueSize());

            opt = queue.GetNextItem(0);
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task3);
            EXPECT_EQ(0, queue.GetQueueSize());

            EXPECT_FALSE(queue.GetNextItem(2));
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

            queue.PushItem(std::move(taskPtr1));
            EXPECT_EQ(1, queue.GetQueueSize());

            queue.Terminate();
            EXPECT_TRUE(queue.IsTerminated());
            EXPECT_FALSE(queue.GetNextItem(0));
            EXPECT_EQ(1, queue.GetQueueSize());
        }
    } // namespace Queues
} // namespace v8App