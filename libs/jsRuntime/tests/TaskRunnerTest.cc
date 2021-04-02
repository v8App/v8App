// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TaskRunner.h"

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

        class MockTaskRunner : public TaskRunner
        {
        public:
            int GetNestingDepth() { return m_NestingDepth; }
            size_t GetQueueSize() { return m_Tasks.size(); }
            bool IsStopped() { return m_Stopped; }
            bool IsNestedTask()
            {
                return m_Tasks.front().first == kNestable;
            }
            size_t GetDelayedQueueSize() { return m_DelayedTasks.size(); }
            bool IsDelayedNestedTask()
            {
                return std::get<1>(m_DelayedTasks.top()) == kNestable;
            }
            size_t GetIdleQueueSize() { return m_IdleTasks.size(); }
            void SetMockTime(double inTime) { m_MockTime = inTime; }

        protected:
            double m_MockTime = 0.0;
            double MonotonicallyIncreasingTime() override
            {
                return m_MockTime;
            }
        };

        TEST(TaskRunnerTest, TaskRunScope)
        {
            std::shared_ptr<MockTaskRunner> runner = std::make_shared<MockTaskRunner>();
            EXPECT_EQ(0, runner->GetNestingDepth());
            {
                TaskRunner::TaskRunScope scope(runner);
                EXPECT_EQ(1, runner->GetNestingDepth());
                {
                    TaskRunner::TaskRunScope scope(runner);
                    EXPECT_EQ(2, runner->GetNestingDepth());
                }
                EXPECT_EQ(1, runner->GetNestingDepth());
            }
            EXPECT_EQ(0, runner->GetNestingDepth());
        }

        TEST(TaskRunnerTest, PostTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();

            TaskPtr task = std::make_unique<RunnerTestTask>();

            EXPECT_EQ(0, runner->GetQueueSize());
            runner->PostTask(std::move(task));
            EXPECT_EQ(nullptr, task.get());
            EXPECT_EQ(1, runner->GetQueueSize());
            EXPECT_TRUE(runner->IsNestedTask());

            //test lock
            {
                std::thread thread1([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostTask(std::move(task));
                    }
                },
                                    runner);
                std::thread thread2([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostTask(std::move(task));
                    }
                },
                                    runner);

                thread1.join();
                thread2.join();

            }

            EXPECT_EQ(21, runner->GetQueueSize());

                //test sopped doesn't queue
                runner->Stop();
                ASSERT_EQ(0, runner->GetQueueSize());
                task = std::make_unique<RunnerTestTask>();
                runner->PostTask(std::move(task));
                ASSERT_EQ(0, runner->GetQueueSize());
        }

        TEST(TaskRunnerTest, PostNonNestableTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();

            TaskPtr task = std::make_unique<RunnerTestTask>();

            EXPECT_EQ(0, runner->GetQueueSize());
            runner->PostNonNestableTask(std::move(task));
            EXPECT_EQ(nullptr, task.get());
            EXPECT_EQ(1, runner->GetQueueSize());
            EXPECT_FALSE(runner->IsNestedTask());

            //test lock
            {
                std::thread thread1([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostNonNestableTask(std::move(task));
                    }
                },
                                    runner);
                std::thread thread2([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostNonNestableTask(std::move(task));
                    }
                },
                                    runner);

                thread1.join();
                thread2.join();
            }

            EXPECT_EQ(21, runner->GetQueueSize());

            //test sopped doesn't queue
            runner->Stop();
            ASSERT_EQ(0, runner->GetQueueSize());
            task = std::make_unique<RunnerTestTask>();
            runner->PostNonNestableTask(std::move(task));
            ASSERT_EQ(0, runner->GetQueueSize());
        }

        TEST(TaskRunnerTest, PostDelayedTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            TaskPtr task = std::make_unique<RunnerTestTask>();

            EXPECT_EQ(0, runner->GetDelayedQueueSize());
            runner->PostDelayedTask(std::move(task), 1.0);
            EXPECT_EQ(nullptr, task.get());
            EXPECT_EQ(1, runner->GetDelayedQueueSize());
            EXPECT_TRUE(runner->IsDelayedNestedTask());

            //test lock
            {
                std::thread thread1([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostDelayedTask(std::move(task), 1.0);
                    }
                },
                                    runner);
                std::thread thread2([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostDelayedTask(std::move(task), 1.0);
                    }
                },
                                    runner);

                thread1.join();
                thread2.join();
            }

            EXPECT_EQ(21, runner->GetDelayedQueueSize());

            //test sopped doesn't queue
            runner->Stop();
            ASSERT_EQ(0, runner->GetDelayedQueueSize());
            task = std::make_unique<RunnerTestTask>();
            runner->PostDelayedTask(std::move(task), 1.0);
            ASSERT_EQ(0, runner->GetDelayedQueueSize());
        }

        TEST(TaskRunnerTest, PostNonNestableDelayedTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            TaskPtr task = std::make_unique<RunnerTestTask>();

            EXPECT_EQ(0, runner->GetDelayedQueueSize());
            runner->PostNonNestableDelayedTask(std::move(task), 1.0);
            EXPECT_EQ(nullptr, task.get());
            EXPECT_EQ(1, runner->GetDelayedQueueSize());
            EXPECT_FALSE(runner->IsDelayedNestedTask());

            //test lock
            {
                std::thread thread1([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostNonNestableDelayedTask(std::move(task), 1.0);
                    }
                },
                                    runner);
                std::thread thread2([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        TaskPtr task = std::make_unique<RunnerTestTask>();
                        runner->PostNonNestableDelayedTask(std::move(task), 1.0);
                    }
                },
                                    runner);

                thread1.join();
                thread2.join();
            }

            EXPECT_EQ(21, runner->GetDelayedQueueSize());

            //test sopped doesn't queue
            runner->Stop();
            ASSERT_EQ(0, runner->GetDelayedQueueSize());
            task = std::make_unique<RunnerTestTask>();
            runner->PostNonNestableDelayedTask(std::move(task), 1.0);
            ASSERT_EQ(0, runner->GetDelayedQueueSize());
        }

        TEST(TaskRunnerTest, PostIdleTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            IdleTaskPtr task = std::make_unique<RunnerTestIdleTask>();

            EXPECT_EQ(0, runner->GetIdleQueueSize());
            runner->PostIdleTask(std::move(task));
            EXPECT_EQ(nullptr, task.get());
            EXPECT_EQ(1, runner->GetIdleQueueSize());

            //test lock
            {
                std::thread thread1([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        IdleTaskPtr task = std::make_unique<RunnerTestIdleTask>();
                        runner->PostIdleTask(std::move(task));
                    }
                },
                                    runner);
                std::thread thread2([](SharedRunner runner) {
                    for (int x = 0; x < 10; x++)
                    {
                        IdleTaskPtr task = std::make_unique<RunnerTestIdleTask>();
                        runner->PostIdleTask(std::move(task));
                    }
                },
                                    runner);

                thread1.join();
                thread2.join();
            }

            EXPECT_EQ(21, runner->GetIdleQueueSize());

            //test sopped doesn't queue
            runner->Stop();
            ASSERT_EQ(0, runner->GetIdleQueueSize());
            task = std::make_unique<RunnerTestIdleTask>();
            runner->PostIdleTask(std::move(task));
            ASSERT_EQ(0, runner->GetIdleQueueSize());
        }
        TEST(TaskRunnerTest, PopTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            TaskPtr task = runner->PopTask();
            EXPECT_EQ(nullptr, task.get());

            task = std::make_unique<RunnerTestTask>();
            //get the pointer to check
            v8::Task *ptr1 = task.get();
            runner->PostTask(std::move(task));
            EXPECT_EQ(nullptr, task.get());
            task = runner->PopTask();
            EXPECT_EQ(ptr1, task.get());

            //test nesting tasks
            //lets put the task back in as a non nestable task
            runner->PostNonNestableTask(std::move(task));
            task = std::make_unique<RunnerTestTask>();
            //get it for comparison
            v8::Task *ptr2 = task.get();
            //post the new task as nestable.
            runner->PostTask(std::move(task));

            //setup a scope and see that we poped then one we just posted.
            {
                TaskRunner::TaskRunScope scope(runner);
                task = runner->PopTask();
                EXPECT_EQ(ptr2, task.get());
                //pop again should get null
                task = runner->PopTask();
                EXPECT_EQ(nullptr, task.get());
            }

            //now we should get teh non nestable one
            task = runner->PopTask();
            EXPECT_EQ(ptr1, task.get());

            //test stopped
            runner->PostTask(std::move(task));
            runner->Stop();
            task = runner->PopTask();
            EXPECT_EQ(nullptr, task.get());
        }

        TEST(TaskRunnerTest, PopIdleTask)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            IdleTaskPtr task = runner->PopIdleTask();
            EXPECT_EQ(nullptr, task.get());

            task = std::make_unique<RunnerTestIdleTask>();
            //get the pointer to check
            v8::IdleTask *ptr1 = task.get();
            runner->PostIdleTask(std::move(task));
            EXPECT_EQ(nullptr, task.get());
            task = runner->PopIdleTask();
            EXPECT_EQ(ptr1, task.get());

            //test stopped
            runner->PostIdleTask(std::move(task));
            runner->Stop();
            task = runner->PopIdleTask();
            EXPECT_EQ(nullptr, task.get());
        }

        TEST(TaskRunnerTest, ProcessDelayedTasks)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();
            //should not do anything
            runner->ProcessDelayedTasks();

            TaskPtr task = std::make_unique<RunnerTestTask>();
            v8::Task *ptr1 = task.get();
            runner->PostDelayedTask(std::move(task), 1.0);
            task = std::make_unique<RunnerTestTask>();
            v8::Task *ptr2 = task.get();
            runner->PostNonNestableDelayedTask(std::move(task), 2.0);
            EXPECT_EQ(2, runner->GetDelayedQueueSize());

            //test nothing ready to move
            runner->ProcessDelayedTasks();
            EXPECT_EQ(2, runner->GetDelayedQueueSize());

            //move time up so one moves
            runner->SetMockTime(1.0);
            runner->ProcessDelayedTasks();
            EXPECT_EQ(1, runner->GetDelayedQueueSize());
            EXPECT_EQ(1, runner->GetQueueSize());

            //move the other one
            runner->SetMockTime(3.0);
            runner->ProcessDelayedTasks();
            EXPECT_EQ(0, runner->GetDelayedQueueSize());
            EXPECT_EQ(2, runner->GetQueueSize());

            //test that teh scope carried over.
            {
                TaskRunner::TaskRunScope scope(runner);
                task = runner->PopTask();
                EXPECT_EQ(ptr1, task.get());
                //pop again should get null
                task = runner->PopTask();
                EXPECT_EQ(nullptr, task.get());
            }

            task = runner->PopTask();
            EXPECT_EQ(ptr2, task.get());

            //test stop
            runner->PostDelayedTask(std::move(task), 1.0);
            runner->Stop();
            runner->ProcessDelayedTasks();
            EXPECT_EQ(0, runner->GetQueueSize());
        }

        TEST(TaskRunnerTest, Stop)
        {
            using SharedRunner = std::shared_ptr<MockTaskRunner>;
            SharedRunner runner = std::make_shared<MockTaskRunner>();

            TaskPtr task = std::make_unique<RunnerTestTask>();
            IdleTaskPtr idleTask = std::make_unique<RunnerTestIdleTask>();
            
            runner->PostIdleTask(std::move(idleTask));
            runner->PostTask(std::move(task));
            task = std::make_unique<RunnerTestTask>();
            runner->PostDelayedTask(std::move(task), 1.0);

            EXPECT_FALSE(runner->IsStopped());
            EXPECT_EQ(1, runner->GetQueueSize());
            EXPECT_EQ(1, runner->GetDelayedQueueSize());
            EXPECT_EQ(1, runner->GetIdleQueueSize());
            runner->Stop();
            EXPECT_TRUE(runner->IsStopped());
            EXPECT_EQ(0, runner->GetQueueSize());
            EXPECT_EQ(0, runner->GetDelayedQueueSize());
            EXPECT_EQ(0, runner->GetIdleQueueSize());
        }
    } // namespace JSRuntime
} // namespace v8App