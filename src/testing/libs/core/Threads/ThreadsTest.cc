// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include <chrono>
#include <future>
#include <iostream>

#if defined(V8APP_WINDOWS)
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Threads/Threads.h"

namespace v8App
{
    namespace Threads
    {
        class TestThread : public Thread
        {
        public:
            TestThread(std::string inName, ThreadPriority inPriority) : Thread(inName, inPriority) {}

        protected:
            virtual void RunImpl() override
            {
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        };

        TEST(ThreadsTest, Constructor)
        {
            std::unique_ptr<TestThread> thread = std::make_unique<TestThread>("test", ThreadPriority::kDefault);

            EXPECT_EQ("test", thread->GetName());
            EXPECT_EQ(ThreadPriority::kDefault, thread->GetPriortiy());
            EXPECT_EQ(-1, thread->GetNativePriority());
        }
        TEST(ThreadsTest, StartJoin)
        {
            std::unique_ptr<TestThread> thread;
#if defined(V8APP_WINDOWS)
            thread = std::make_unique<TestThread>("test", ThreadPriority::kUserBlocking);
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
            thread = std::make_unique<TestThread>("test", ThreadPriority::kBestEffort);
#endif
            EXPECT_EQ(-1, thread->GetNativePriority());
            thread->Start();
            // need to yeild apparently to give the OS time to set stuff
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_EQ("test", thread->GetNativeName());
#if defined(V8APP_WINDOWS)
            EXPECT_EQ(ThreadPriority::kUserBlocking, thread->GetPriortiy());
            EXPECT_EQ(THREAD_PRIORITY_TIME_CRITICAL, thread->GetNativePriority());
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
            EXPECT_EQ(ThreadPriority::kBestEffort, thread->GetPriortiy());
            EXPECT_EQ(QOS_CLASS_BACKGROUND, thread->GetNativePriority());
#endif
            thread->Join();

            // when the thread is joined and we come back the native thread functions won't work so test
            thread = std::make_unique<TestThread>("test", ThreadPriority::kUserBlocking);
            thread->Start();
            thread->Join();
            EXPECT_EQ("", thread->GetNativeName());
            EXPECT_EQ(-1, thread->GetNativePriority());
        }
    }
}