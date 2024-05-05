// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ForegroundTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        class RunnerTestTask : public v8::Task
        {
            void Run() override
            {
            }
        };

        class RunnerTestIdleTask : public v8::IdleTask
        {
            void Run(double time) override
            {
            }
        };

        class MockTaskRunner : public ForegroundTaskRunner
        {
        public:
            MockTaskRunner() {}

            int GetNestingDepth() { return m_NestingDepth; }
            bool IsTerminated() { return m_Terminated; }
        };

        TEST(ForegroundTaskRunnerTest, Constrcutor)
        {
            MockTaskRunner runner;
            EXPECT_FALSE(runner.IsTerminated());
            EXPECT_TRUE(runner.NonNestableTasksEnabled());
            EXPECT_TRUE(runner.NonNestableDelayedTasksEnabled());
            EXPECT_TRUE(runner.IdleTasksEnabled());
        }

        TEST(ForegroundTaskRunnerTest, TaskRunScope)
        {
            std::shared_ptr<MockTaskRunner> runner = std::make_shared<MockTaskRunner>();
            EXPECT_EQ(0, runner->GetNestingDepth());
            {
                ForegroundTaskRunner::TaskRunScope scope(runner);
                EXPECT_EQ(1, runner->GetNestingDepth());
                {
                    ForegroundTaskRunner::TaskRunScope scope(runner);
                    EXPECT_EQ(2, runner->GetNestingDepth());
                }
                EXPECT_EQ(1, runner->GetNestingDepth());
            }
            EXPECT_EQ(0, runner->GetNestingDepth());
        }

        TEST(ForegroundTaskRunnerTest, Tasks)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();

            V8TaskUniquePtr task1 = std::make_unique<RunnerTestTask>();
            V8TaskUniquePtr task2 = std::make_unique<RunnerTestTask>();
            V8TaskUniquePtr task3 = std::make_unique<RunnerTestTask>();
            V8TaskUniquePtr task4 = std::make_unique<RunnerTestTask>();

            v8::Task *ptask1 = task1.get();
            v8::Task *ptask2 = task2.get();
            v8::Task *ptask3 = task3.get();
            v8::Task *ptask4 = task4.get();

            TestTime::TestTimeSeconds::Enable();
            TestTime::TestTimeSeconds::Set(0);

            EXPECT_FALSE(runner->MaybeHasTask());
            runner->PostNonNestableTask(std::move(task1));
            runner->PostNonNestableDelayedTask(std::move(task2), 4.0);
            runner->PostTask(std::move(task3));
            runner->PostDelayedTask(std::move(task4), 4.0);
            EXPECT_TRUE(runner->MaybeHasTask());

            V8TaskUniquePtr opt;
            {
                ForegroundTaskRunner::TaskRunScope scope(runner);

                opt = runner->GetNextTask();
                EXPECT_NE(opt, nullptr);
                EXPECT_EQ(opt.get(), ptask3);

                opt = runner->GetNextTask();
                EXPECT_EQ(opt, nullptr);

            TestTime::TestTimeSeconds::Set(5);

                opt = runner->GetNextTask();
                EXPECT_NE(opt, nullptr);
                EXPECT_EQ(opt.get(), ptask4);

                EXPECT_TRUE(runner->MaybeHasTask());
                opt = runner->GetNextTask();
                EXPECT_EQ(opt, nullptr);
            }
            opt = runner->GetNextTask();
            EXPECT_NE(opt, nullptr);
            EXPECT_EQ(opt.get(), ptask1);

            opt = runner->GetNextTask();
            EXPECT_NE(opt, nullptr);
            EXPECT_EQ(opt.get(), ptask2);
            EXPECT_FALSE(runner->MaybeHasTask());
        }

        TEST(ForegroundTaskRunnerTest, IdleTasks)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            V8IdleTaskUniquePtr task1 = std::make_unique<RunnerTestIdleTask>();
            V8IdleTaskUniquePtr task2 = std::make_unique<RunnerTestIdleTask>();

            v8::IdleTask *ptask1 = task1.get();
            v8::IdleTask *ptask2 = task2.get();

            V8IdleTaskUniquePtr opt;

            EXPECT_FALSE(runner->MaybeHasIdleTask());
            opt = runner->GetNextIdleTask();
            EXPECT_EQ(opt, nullptr);

            runner->PostIdleTask(std::move(task1));
            runner->PostIdleTask(std::move(task2));
            EXPECT_TRUE(runner->MaybeHasIdleTask());

            opt = runner->GetNextIdleTask();
            EXPECT_NE(opt, nullptr);
            EXPECT_EQ(opt.get(), ptask1);
            EXPECT_TRUE(runner->MaybeHasIdleTask());

            opt = runner->GetNextIdleTask();
            EXPECT_NE(opt, nullptr);
            EXPECT_EQ(opt.get(), ptask2);
            EXPECT_FALSE(runner->MaybeHasIdleTask());
        }

        TEST(ForegroundTaskRunnerTest, Terminates)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();

            V8TaskUniquePtr task1 = std::make_unique<RunnerTestTask>();
            V8IdleTaskUniquePtr idleTask1 = std::make_unique<RunnerTestIdleTask>();

            EXPECT_FALSE(runner->IsTerminated());
            EXPECT_FALSE(runner->MaybeHasTask());
            EXPECT_FALSE(runner->MaybeHasIdleTask());

            runner->Terminate();

            EXPECT_TRUE(runner->IsTerminated());
            runner->PostTask(std::move(task1));
            runner->PostIdleTask(std::move(idleTask1));
            EXPECT_FALSE(runner->MaybeHasIdleTask());
        }
    } // namespace JSRuntime
} // namespace v8App