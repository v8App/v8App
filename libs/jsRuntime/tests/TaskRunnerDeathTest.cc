// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        class RunnerDeathTestTask : public v8::Task
        {
            void Run() override
            {
            }
        };

        class MockDeathTaskRunner : public TaskRunner
        {
        public:
            void SetNestingDepth(int inDepth) { m_NestingDepth = inDepth; }
        };

        TEST(TaskRunnerDeathTest, TaskRunScope)
        {
//Only applicable in debug builds
#if V8APP_DEBUG
            ASSERT_DEATH({
                std::shared_ptr<MockDeathTaskRunner> runner = std::make_shared<MockDeathTaskRunner>();
                runner->SetNestingDepth(-1);
                TaskRunner::TaskRunScope scope(runner);
            },
                         "v8App Log {");

            ASSERT_DEATH({
                std::shared_ptr<MockDeathTaskRunner> runner = std::make_shared<MockDeathTaskRunner>();
                {
                    TaskRunner::TaskRunScope scope(runner);
                    runner->SetNestingDepth(-1);
                }
            },
                         "v8App Log {");
#endif
        }
        TEST(TaskRunnerDeathTest, PostDelayedTask)
        {
#if V8APP_DEBUG
            ASSERT_DEATH({
                TaskRunner runner;
                TaskPtr task = std::make_unique<RunnerDeathTestTask>();
                runner.PostDelayedTask(std::move(task), -1.0);
            },
                         "v8App Log {");
#endif
        }
        TEST(TaskRunnerDeathTest, PostNonNestableDelayedTask)
        {
#if V8APP_DEBUG
            ASSERT_DEATH({
                TaskRunner runner;
                TaskPtr task = std::make_unique<RunnerDeathTestTask>();
                runner.PostNonNestableDelayedTask(std::move(task), -1.0);
            },
                         "v8App Log {");
#endif
        }
    } // namespace JSRuntime
} // namespace v8App