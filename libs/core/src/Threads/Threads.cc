// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <format>

#if defined(V8APP_WINDOWS)
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
#include <pthread.h>
#endif

#include "Threads/Threads.h"
#include "Logging/LogMacros.h"

namespace v8App
{
    namespace Threads
    {
        void SetThreadPriority(std::thread *inThread, ThreadPriority inPriority)
        {
            bool succeeded = true;
#if defined(V8APP_WINDOWS)
            int priority = THREAD_PRIORITY_TIME_CRITICAL; // kUserBlocking
            switch (inPriority)
            {
            case ThreadPriority::kBestEffort:
                priority = THREAD_PRIORITY_NORMAL;
                break;
            case ThreadPriority::kUserVisible:
                priority = THREAD_PRIORITY_ABOVE_NORMAL;
                break;
            }
            succeeded = ::SetThreadPriority(inThread->native_handle(), priority) != 0;
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
            int priority = 10; // kUserBlocking
            switch (inPriority)
            {
            case ThreadPriority::kBestEffort:
                priority = 5;
                break;
            case ThreadPriority::kUserVisible:
                priority = 8;
                break;
            }
            sched_param params;
            params.sched_priority = priority;
            succeeded = pthread_setschedparam(inThread->native_handle(), SCHED_RR, priority) == 0;
#endif
            if (succeeded == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, std::format("Failed to set thread priorty"));
                LOG_WARN(msg);
            }
        }

        int GetThreadPriority(std::thread *inThread)
        {
#if defined(V8APP_WINDOWS)
            return ::GetThreadPriority(inThread->native_handle());
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
            int policy;
            sched_param param;
            pthread_getschedparam(inThread->native_handle(), &policy, &param);
            return param.sched_priority;
#endif
        }
    }
} // namespace v8App
