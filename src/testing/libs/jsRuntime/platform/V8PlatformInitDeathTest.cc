
// Copyright 2020 The v8App Authors. All rights reserved.
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
        class TestInitIsolateHelper : public PlatformIsolateHelper
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority priority) override
            {
                return std::shared_ptr<v8::TaskRunner>();
            };
            virtual bool IdleTasksEnabled(v8::Isolate *inIsolate) override { return true; }
        };

        class TestV8Platform : public V8Platform
        {
        public:
            TestV8Platform() {}

            Threads::ThreadPriority TestIntToPriority(int inInt) { return IntToPriority(inInt); }
            int TestPriorityToInt(v8::TaskPriority inPriority) { return PriorityToInt(inPriority); }
            bool IsInited() { return s_PlatformInited; }
            void SetInited(bool inValue) { s_PlatformInited = inValue; }
            bool IsDestoryed() { return s_PlatformDestoryed; }
            void SetDestoryed(bool inValue) { s_PlatformDestoryed = inValue; }
        };

        TEST(V8PlatformInitDeathTest, InitializeShutdownV8StaticNull)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_EXIT({
                PlatformIsolateHelperUniquePtr helper = std::make_unique<TestInitIsolateHelper>();
                V8Platform::InitializeV8(std::move(helper));
                std::shared_ptr<V8Platform> platform = V8Platform::Get();

                std::unique_ptr<v8::HighAllocationThroughputObserver> observer = std::make_unique<v8::HighAllocationThroughputObserver>();
                platform->SetHighAllocatoionObserver(observer.get());
                EXPECT_EQ(observer.get(), platform->GetHighAllocationThroughputObserver());
                observer.release();
                V8Platform::ShutdownV8();
                EXPECT_NE(platform, V8Platform::Get());

                std::exit(0);
            },
                        ::testing::ExitedWithCode(0), "");
        }

        TEST(V8PlatformInitDeathTest, InitializeShutdownV8GetCalledBeforeInit)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_EXIT({
                PlatformIsolateHelperUniquePtr helper = std::make_unique<TestInitIsolateHelper>();
                V8Platform::InitializeV8(std::move(helper));
                std::shared_ptr<V8Platform> platform = V8Platform::Get();

                std::unique_ptr<v8::HighAllocationThroughputObserver> observer = std::make_unique<v8::HighAllocationThroughputObserver>();
                platform->SetHighAllocatoionObserver(observer.get());
                EXPECT_EQ(observer.get(), platform->GetHighAllocationThroughputObserver());
                observer.release();
                V8Platform::ShutdownV8();
                EXPECT_NE(platform, V8Platform::Get());
                //sould just reutrn and thus not cause any issue
                V8Platform::ShutdownV8();
                std::exit(0);
            },
                        ::testing::ExitedWithCode(0), "");
        }
    } // namespace JSRuntime
} // namespace v8App