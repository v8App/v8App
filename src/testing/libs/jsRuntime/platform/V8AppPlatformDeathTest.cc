
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "V8AppPlatform.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestDeathIsolateHelper : public PlatformIsolateHelper
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) override
            {
                return V8TaskRunnerSharedPtr();
            };
            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) override { return true; }
        };

        TEST(V8AppPlatformDeathTest, GetForgroundTaskRunnerHelperNull)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();
                V8TaskRunnerSharedPtr runner = platform->GetForegroundTaskRunner(nullptr, V8TaskPriority::kBestEffort);
                std::exit(0);
            },
                         "");
        }

        TEST(V8AppPlatformDeathTest, IdleTaskEnabledHelperNull)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();
                platform->IdleTasksEnabled(nullptr);
                std::exit(0);
            },
                         "");
        }

        TEST(V8AppPlatformDeathTest, InitAfterDestory)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_EXIT({
                PlatformIsolateHelperUniquePtr helper = std::make_unique<TestDeathIsolateHelper>();
                V8AppPlatform::InitializeV8(std::move(helper));
                std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();

                V8HighAllocationThroughputObserverUniquePtr observer = std::make_unique<V8HighAllocationThroughputObserver>();
                platform->SetHighAllocatoionObserver(observer.get());
                EXPECT_EQ(observer.get(), platform->GetHighAllocationThroughputObserver());
                observer.release();
                V8AppPlatform::ShutdownV8();
                V8AppPlatform::InitializeV8(std::move(helper));

                std::exit(0);
            },
                        testing::ExitedWithCode(0), "");
        }
    } // namespace JSRuntime
} // namespace v8App