// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WorkerTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        class WorkerRunnerTestTask : public v8::Task
        {
        public:
            explicit WorkerRunnerTestTask(int *inInt, int inValue) : m_Int(inInt), m_Value(inValue) {}
            void Run() override
            {
                *m_Int = m_Value;
            }
            int *m_Int;
            int m_Value;
        };

        class MockWorkerTaskRunner : public WorkerTaskRunner
        {
        public:
            MockWorkerTaskRunner(size_t inNumWorkers, Threads::ThreadPriority inPriority) : WorkerTaskRunner(inNumWorkers, inPriority) {}
            bool IsTerminated() { return m_Terminated; }
        };

        TEST(WorkerTaskRunnerTest, Constrcutor)
        {
            MockWorkerTaskRunner runner(1, Threads::ThreadPriority::kBestEffort);
            EXPECT_FALSE(runner.IsTerminated());
            EXPECT_FALSE(runner.NonNestableTasksEnabled());
            EXPECT_FALSE(runner.NonNestableDelayedTasksEnabled());
            EXPECT_FALSE(runner.IdleTasksEnabled());
        }

        TEST(WorkerTaskRunnerTest, Tasks)
        {
            using SharedRunner = std::shared_ptr<MockWorkerTaskRunner>;
            SharedRunner runner = std::make_shared<MockWorkerTaskRunner>(4, Threads::ThreadPriority::kBestEffort);
            int task1Int = 0;
            int task2Int = 0;
            int task3Int = 0;
            int task4Int = 0;

            V8TaskUniquePtr task1 = std::make_unique<WorkerRunnerTestTask>(&task1Int, 1);
            V8TaskUniquePtr task2 = std::make_unique<WorkerRunnerTestTask>(&task2Int, 2);
            V8TaskUniquePtr task3 = std::make_unique<WorkerRunnerTestTask>(&task3Int, 3);
            V8TaskUniquePtr task4 = std::make_unique<WorkerRunnerTestTask>(&task4Int, 4);

            TestTime::TestTimeSeconds::Enable();
            TestTime::TestTimeSeconds::Set(0.0);

            runner->PostTask(std::move(task1));
            runner->PostDelayedTask(std::move(task2), 4.0);
            runner->PostNonNestableTask(std::move(task3));
            runner->PostNonNestableDelayedTask(std::move(task4), 4.0);

            std::this_thread::sleep_for(std::chrono::seconds(2));

            // only task 1 should have been run task 2 is delayed and nestable are not supo
            EXPECT_EQ(1, task1Int);
            EXPECT_EQ(0, task2Int);
            EXPECT_EQ(0, task3Int);
            EXPECT_EQ(0, task4Int);

            TestTime::TestTimeSeconds::Set(6.0);

            // task 2 should get executed now
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(1, task1Int);
            EXPECT_EQ(2, task2Int);
            EXPECT_EQ(0, task3Int);
            EXPECT_EQ(0, task4Int);

            // shold be no change to any
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(1, task1Int);
            EXPECT_EQ(2, task2Int);
            EXPECT_EQ(0, task3Int);
            EXPECT_EQ(0, task4Int);
        }

        TEST(WorkerTaskRunnerTest, Terminates)
        {
            using SharedRunner = std::shared_ptr<MockWorkerTaskRunner>;
            SharedRunner runner = std::make_shared<MockWorkerTaskRunner>(1, Threads::ThreadPriority::kBestEffort);

            int task1Int = 0;
            V8TaskUniquePtr task1 = std::make_unique<WorkerRunnerTestTask>(&task1Int, 1);

            EXPECT_FALSE(runner->IsTerminated());

            runner->Terminate();

            EXPECT_TRUE(runner->IsTerminated());
            runner->PostTask(std::move(task1));

            //since it's terminated the task should not have posted and ran
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(0, task1Int);
        }
    } // namespace JSRuntime
} // namespace v8App