// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <format>

#include "Threads/ThreadPoolQueue.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"
        
namespace v8App
{
    namespace Threads
    {
        ThreadPoolQueue::ThreadPoolQueue(int inNumberOfWorkers, ThreadPriority inPriority) : m_Priority(inPriority)
        {
            // We want to leave one core for the main thread.
            int hardwareThreads = GetHardwareCores();
            if (inNumberOfWorkers < 0)
            {
                inNumberOfWorkers = hardwareThreads;
            }
            m_NumWorkers = std::max(1, std::min(inNumberOfWorkers, hardwareThreads));
            for (int x = 0; x < m_NumWorkers; x++)
            {
                std::string name = Utils::format("ThreadPool #{}", x);
                std::unique_ptr<Thread> thread = std::make_unique<ThreadPoolThread>(name, m_Priority, this);
                thread->Start();
                m_Workers.push_back(std::move(thread));
            }
        }

        ThreadPoolQueue::~ThreadPoolQueue()
        {
            Terminate();
        }

        bool ThreadPoolQueue::PostTask(ThreadPoolTaskUniquePtr inTask)
        {
            // if we are in the process of exiting then return
            if (m_Exiting)
            {
                return false;
            }

            m_Queue.PushItem(std::move(inTask));
            m_QueueWaiter.notify_one();
            return true;
        }

        void ThreadPoolQueue::Terminate()
        {
            if (m_Exiting)
            {
                return;
            }
            // if the m_Exiting hasn't been set yet then set it
            if (m_Exiting == false)
            {
                m_Exiting.exchange(true);
            }
            m_QueueWaiter.notify_all();
            for (auto &it : m_Workers)
            {
                it->Join();
            }
            m_Workers.clear();
        }

        void ThreadPoolQueue::ProcessTasks()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_QueueLock);
                m_QueueWaiter.wait(lock, [=]()
                                   { return m_Exiting == true || m_Queue.MayHaveItems(); });
                if (m_Exiting)
                {
                    return;
                }

                std::optional<ThreadPoolTaskUniquePtr> task = m_Queue.GetNextItem();
                if (task)
                {
                    task.value()->Run();
                }
            }
        }
    } // namespace ThreadPool
} // namespace v8App