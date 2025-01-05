// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _THREADS_H__
#define _THREADS_H__

#include <thread>
#include <string>
#include <mutex>

namespace v8App
{
    namespace Threads
    {
        /**
         * These mirror the v8 Thread Priorities
         */
        enum class ThreadPriority : uint8_t
        {
            kDefault,
            kBestEffort,
            kUserVisible,
            kUserBlocking,
            kMaxPriority = kUserBlocking
        };

        /**
         * The max size of a therad name
        */
        constexpr int kMaxThreadName = 63;

        /**
         * class to implement a thread. To implment the logic for the thread 
         * Implment the Run fucntion
        */
        class Thread
        {
        public:
            Thread(std::string inName, ThreadPriority inPriority = ThreadPriority::kDefault);
            virtual ~Thread();

            Thread(const Thread &) = delete;
            Thread &operator=(const Thread &) = delete;

            void Start();

            const std::string GetName() { return m_Name; }
            enum ThreadPriority GetPriortiy() { return m_Priority; }
            int GetNativePriority();
            std::string GetNativeName();

            void Join();

        protected:
            virtual void RunImpl() = 0;
            void SetThreadPriority();
            void SetThreadName();

            std::string m_Name;
            ThreadPriority m_Priority;
            std::unique_ptr<std::thread> m_Thread;
            bool m_Running = false;
            std::mutex m_Lock;
        };

        /**
         * Gets the systems number of harware cores
        */
        int GetHardwareCores();
    } // namespace Threads
} // namespace v8App

#endif //_THREADS_H__