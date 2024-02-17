// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ForegroundTaskRunner.h"

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

        class MockDeathTaskRunner : public ForegroundTaskRunner
        {
        public:
            void SetNestingDepth(int inDepth) { m_NestingDepth = inDepth; }
        };

        TEST(ForegroundTaskRunnerDeathTest, TaskRunScope)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            // Only applicable in debug builds
#ifdef V8APP_DEBUG
            ASSERT_DEATH({
                std::shared_ptr<MockDeathTaskRunner> runner = std::make_shared<MockDeathTaskRunner>();
                runner->SetNestingDepth(-1);
                ForegroundTaskRunner::TaskRunScope scope(runner);
                std::exit(0);
            },
                         "");

            EXPECT_EXIT({
                std::shared_ptr<MockDeathTaskRunner> runner = std::make_shared<MockDeathTaskRunner>();
                {
                    ForegroundTaskRunner::TaskRunScope scope(runner);
                    runner->SetNestingDepth(-1);
                    std::exit(0);
                }
            },
                        ::testing::ExitedWithCode(0), "");
#endif
        }
    } // namespace JSRuntime
} // namespace v8App