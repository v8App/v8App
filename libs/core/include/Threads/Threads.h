// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _THREADS_H__
#define _THREADS_H__

#include <thread>

namespace v8App
{
    namespace Threads
    {
        // These mirror the v8 Thread Priorities
        enum class ThreadPriority : uint8_t
        {
            kBestEffort,
            kUserVisible,
            kUserBlocking,
            kMaxPriority = kUserBlocking
        };

        // allows setting the std::thread's native priority
        void SetThreadPriority(std::thread *inThread, ThreadPriority inPriority);
        //allows getting the native thread priority from the std::thread
        int GetThreadPriority(std::thread *inThread);

        int GetHardwareCores();
    } // namespace Threads
} // namespace v8App

#endif //_THREADS_H__