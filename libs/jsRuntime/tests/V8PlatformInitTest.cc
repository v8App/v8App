
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
        class TestV8Platform : public V8Platform
        {
        public:
            TestV8Platform() {}

            Threads::ThreadPriority TestIntToPriority(int inInt) { return IntToPriority(inInt); }
            int TestPriorityToInt(v8::TaskPriority inPriority) { return PriorityToInt(inPriority); }
            bool IsInited() { return m_V8Inited; }
            void SetInited(bool inValue) { m_V8Inited = inValue; }
        };

        TEST(V8PlatformInitTest, InitializeShutdownV8StaticNull)
        {
            EXPECT_EXIT({
                V8Platform::InitializeV8();
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

        TEST(V8PlatformInitTest, InitializeShutdownV8GetCalledBeforeInit)
        {
            EXPECT_EXIT({
                std::shared_ptr<V8Platform> platform = V8Platform::Get();
                V8Platform::InitializeV8();

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
    } // namespace JSRuntime
} // namespace v8App