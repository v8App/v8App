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
        TEST(ThreadsTest, SetGetThreadPriority)
        {
            std::thread thread([]() {});

#if defined(V8APP_WINDOWS)
            EXPECT_EQ(THREAD_PRIORITY_NORMAL, GetThreadPriority(&thread));
            SetThreadPriority(&thread, ThreadPriority::kUserBlocking);
            EXPECT_EQ(THREAD_PRIORITY_TIME_CRITICAL, GetThreadPriority(&thread));
#elif defined(V8APP_MACOS)
           EXPECT_EQ(0, GetThreadPriority(&thread));
            SetThreadPriority(&thread, ThreadPriority::kUserBlocking);
            EXPECT_EQ(10, GetThreadPriority(&thread));
#endif
            // hasve to join it or it call terminate and kills the tests
            thread.join();
            std::cout << "Finished Test" << std::endl;
        }
    }
}