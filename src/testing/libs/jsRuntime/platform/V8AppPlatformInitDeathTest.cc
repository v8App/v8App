
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8AppPlatform.h"
#include "IJSPlatformRuntimeProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestInitIsolateHelper : public IJSPlatformRuntimeProvider
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) override
            {
                return V8TaskRunnerSharedPtr();
            };
            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) override { return true; }
        };

        class TestV8AppPlatform : public V8AppPlatform
        {
        public:
            TestV8AppPlatform() {}

            Threads::ThreadPriority TestIntToPriority(int inInt) { return IntToPriority(inInt); }
            int TestPriorityToInt(V8TaskPriority inPriority) { return PriorityToInt(inPriority); }
            bool IsInited() { return s_PlatformInited; }
            void SetInited(bool inValue) { s_PlatformInited = inValue; }
            bool IsDestoryed() { return s_PlatformDestroyed; }
            void SetDestoryed(bool inValue) { s_PlatformDestroyed = inValue; }
        };

        TEST(V8AppPlatformInitDeathTest, InitializeShutdownV8StaticNull)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_EXIT({
                PlatformRuntimeProviderUniquePtr helper = std::make_unique<TestInitIsolateHelper>();
                V8AppPlatform::InitializeV8(std::move(helper));
                std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();

                V8HighAllocationThroughputObserverUniquePtr observer = std::make_unique<V8HighAllocationThroughputObserver>();
                platform->SetHighAllocatoionObserver(observer.get());
                EXPECT_EQ(observer.get(), platform->GetHighAllocationThroughputObserver());
                observer.release();
                V8AppPlatform::ShutdownV8();
                EXPECT_NE(platform, V8AppPlatform::Get());

                std::exit(0);
            },
                        ::testing::ExitedWithCode(0), "");
        }

        TEST(V8AppPlatformInitDeathTest, InitializeShutdownV8GetCalledBeforeInit)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_EXIT({
                PlatformRuntimeProviderUniquePtr helper = std::make_unique<TestInitIsolateHelper>();
                V8AppPlatform::InitializeV8(std::move(helper));
                std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();

                V8HighAllocationThroughputObserverUniquePtr observer = std::make_unique<V8HighAllocationThroughputObserver>();
                platform->SetHighAllocatoionObserver(observer.get());
                EXPECT_EQ(observer.get(), platform->GetHighAllocationThroughputObserver());
                observer.release();
                V8AppPlatform::ShutdownV8();
                EXPECT_NE(platform, V8AppPlatform::Get());
                //sould just reutrn and thus not cause any issue
                V8AppPlatform::ShutdownV8();
                std::exit(0);
            },
                        ::testing::ExitedWithCode(0), "");
        }
    } // namespace JSRuntime
} // namespace v8App