// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Queues/TThreadSafeQueue.h"

namespace v8App
{
    namespace Queues
    {
        class TestQueueTask
        {
        public:
            ~TestQueueTask() {}

            void Run()
            {
            }
        };

        using TaskTaskUniquePtr = std::unique_ptr<TestQueueTask>;

        class TestTaskQueue : public TThreadSafeQueue<TaskTaskUniquePtr>
        {
        public:
            size_t GetQueueSize() { return m_Queue.size(); }
            bool IsTerminated() { return m_Terminated; }
        };

        TEST(TThreadSafeQueue, Constrcutor)
        {
            TestTaskQueue queue = TestTaskQueue();

            EXPECT_EQ(false, queue.IsTerminated());
            EXPECT_EQ(0, queue.GetQueueSize());
        }

        TEST(TThreadSafeQueue, PushItemGetNextItem)
        {
            TestTaskQueue queue = TestTaskQueue();
            TaskTaskUniquePtr taskPtr1 = std::make_unique<TestQueueTask>();
            TaskTaskUniquePtr taskPtr2 = std::make_unique<TestQueueTask>();
            TestQueueTask *task1 = taskPtr1.get();
            TestQueueTask *task2 = taskPtr2.get();

            EXPECT_FALSE(queue.GetNextItem());
            queue.PushItem(std::move(taskPtr1));
            EXPECT_EQ(1, queue.GetQueueSize());

            queue.PushItem(std::move(taskPtr2));
            EXPECT_EQ(2, queue.GetQueueSize());

            auto opt = queue.GetNextItem();
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task1);

            opt = queue.GetNextItem();
            EXPECT_TRUE(opt.has_value());
            ASSERT_EQ(opt.value().get(), task2);

            EXPECT_FALSE(queue.GetNextItem());
        }
        
        TEST(TThreadSafeQueue, MayHaveItems)
        {
            TestTaskQueue queue = TestTaskQueue();
            TaskTaskUniquePtr taskPtr1 = std::make_unique<TestQueueTask>();
            
            EXPECT_FALSE(queue.MayHaveItems());
            queue.PushItem(std::move(taskPtr1));
            EXPECT_TRUE(queue.MayHaveItems());
        }

        TEST(TThreadSafeQueue, Terminates)
        {
            TestTaskQueue queue = TestTaskQueue();
            TaskTaskUniquePtr taskPtr1 = std::make_unique<TestQueueTask>();
            TaskTaskUniquePtr taskPtr2 = std::make_unique<TestQueueTask>();

            queue.PushItem(std::move(taskPtr1));
            EXPECT_EQ(1, queue.GetQueueSize());

            queue.Terminate();
            EXPECT_TRUE(queue.IsTerminated());
            EXPECT_FALSE(queue.GetNextItem());
            EXPECT_EQ(1, queue.GetQueueSize());
            queue.PushItem(std::move(taskPtr2));
            EXPECT_EQ(1, queue.GetQueueSize());

        }
    }
}
