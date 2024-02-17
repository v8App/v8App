
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "V8Platform.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestDeathIsolateHelper : public PlatformIsolateHelper
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority priority) override
            {
                return std::shared_ptr<v8::TaskRunner>();
            };
            virtual bool IdleTasksEnabled(v8::Isolate *inIsolate) override { return true; }
        };

        TEST(V8PlatformDeathTest, GetForgroundTaskRunnerHelperNull)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<V8Platform> platform = V8Platform::Get();
                V8TaskRunnerSharedPtr runner = platform->GetForegroundTaskRunner(nullptr, v8::TaskPriority::kBestEffort);
                std::exit(0);
            },
                         "");
        }

        TEST(V8PlatformDeathTest, IdleTaskEnabledHelperNull)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::shared_ptr<V8Platform> platform = V8Platform::Get();
                platform->IdleTasksEnabled(nullptr);
                std::exit(0);
            },
                         "");
        }

        TEST(V8PlatformDeathTest, InitAfterDestory)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_EXIT({
                PlatformIsolateHelperUniquePtr helper = std::make_unique<TestDeathIsolateHelper>();
                V8Platform::InitializeV8(std::move(helper));
                std::shared_ptr<V8Platform> platform = V8Platform::Get();

                std::unique_ptr<v8::HighAllocationThroughputObserver> observer = std::make_unique<v8::HighAllocationThroughputObserver>();
                platform->SetHighAllocatoionObserver(observer.get());
                EXPECT_EQ(observer.get(), platform->GetHighAllocationThroughputObserver());
                observer.release();
                V8Platform::ShutdownV8();
                V8Platform::InitializeV8(std::move(helper));

                std::exit(0);
            },
                        testing::ExitedWithCode(0), "");
        }
    } // namespace JSRuntime
} // namespace v8App