// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Threads/Threads.h"
#include <format>

#if defined(V8APP_WINDOWS)
#include <windows.h>
#include <winerror.h>
#include <processthreadsapi.h>
#include <codecvt>
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
#include <pthread.h>
#endif

#include "Threads/Threads.h"
#include "Logging/LogMacros.h"

namespace v8App
{
    namespace Threads
    {
        Thread::Thread(std::string inName, ThreadPriority inPriority) : m_Name(inName), m_Priority(inPriority)
        {
            if (m_Name.length() > kMaxThreadName)
            {
                m_Name.resize(kMaxThreadName);
            }
        }

        Thread::~Thread()
        {
            Join();
        }

        void Thread::Start()
        {
            std::lock_guard<std::mutex> lock(m_Lock);
            if (m_Running)
            {
                return;
            }
            m_Thread = std::make_unique<std::thread>([this]()
                                                     {
                {
                    //we use a mutex to prevent the thread from precedding till after
                    // the Start call has finished due to m_Thread being null still when
                    // it some code that expects it to be set and needs to be set. 
                    std::lock_guard<std::mutex> lock(m_Lock);
                    this->m_Running = true;
                    this->SetThreadPriority();
                    this->SetThreadName();
                }
                this->RunImpl();
                this->m_Running = false; });
        }

        void Thread::Join()
        {
            if (m_Thread != nullptr && m_Thread->joinable())
            {
                m_Thread->join();
            }
        }

        int Thread::GetNativePriority()
        {
            if (m_Thread == nullptr || m_Running == false)
            {
                return -1;
            }
#if defined(V8APP_WINDOWS)
            return ::GetThreadPriority(m_Thread->native_handle());
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
            qos_class_t policy;
            int priorty;
            pthread_get_qos_class_np(m_Thread->native_handle(), &policy, &priorty);
            return policy;
#endif
        }

        std::string Thread::GetNativeName()
        {
            if (m_Thread != nullptr && m_Running)
            {
#if defined(V8APP_WINDOWS)
                PWSTR name;
                HRESULT result = ::GetThreadDescription(m_Thread->native_handle(), &name);
                if (SUCCEEDED(result))
                {
                    std::wstring wtemp(name);
                    // NOTE: codecvt is being removed in c++26 so when we get to that point we'll
                    // have to convert this to what ever replaces it.
                    std::string temp = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wtemp);
                    LocalFree(name);

                    return temp;
                }
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
                char temp[kMaxThreadName + 1];

                if (pthread_getname_np(m_Thread->native_handle(), temp, kMaxThreadName) == 0)
                {
                    return std::string(temp);
                }
#endif
            }
            return std::string();
        }

        void Thread::SetThreadPriority()
        {
            bool succeeded = true;
            if (std::this_thread::get_id() != m_Thread->get_id())
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Tried to se the thread priorty for this thread on a different thread");
                LOG_WARN(msg);

                succeeded = false;
            }
            else
            {
#if defined(V8APP_WINDOWS)
                int priority = -1; // kUserBlocking
                switch (m_Priority)
                {
                case ThreadPriority::kBestEffort:
                    priority = THREAD_PRIORITY_NORMAL;
                    break;
                case ThreadPriority::kUserVisible:
                    priority = THREAD_PRIORITY_ABOVE_NORMAL;
                    break;
                case ThreadPriority::kUserBlocking:
                    priority = THREAD_PRIORITY_TIME_CRITICAL;
                default:
                    succeeded = true;
                }
                if (priority != -1)
                {
                    succeeded = ::SetThreadPriority(m_Thread->native_handle(), priority) != 0;
                }
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
                switch (m_Priority)
                {
                case ThreadPriority::kBestEffort:
                    succeeded = pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0) == 0;
                    break;
                case ThreadPriority::kUserVisible:
                    succeeded = pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, -1) == 0;
                    break;
                case ThreadPriority::kUserBlocking:
                    succeeded = pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0) == 0;
                    break;
                default:
                    succeeded = true;
                }
#endif
            }
            if (succeeded == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to set thread priorty");
                LOG_WARN(msg);
            }
        }

        void Thread::SetThreadName()
        {
#if defined(V8APP_WINDOWS)
            std::wstring temp = std::wstring(m_Name.begin(), m_Name.end());
            SetThreadDescription(m_Thread->native_handle(), temp.c_str());
#elif defined(V8APP_MACOS) || defined(V8APP_IOS)
            pthread_setname_np(m_Name.c_str());
#endif
        }

        int GetHardwareCores()
        {
            return std::min(1U, std::thread::hardware_concurrency() - 1);
        }

    }
} // namespace v8App
