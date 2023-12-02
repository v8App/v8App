// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <format>
#include <iostream>
#include <functional>

#include "Threads/ThreadPoolDelayedQueue.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"

namespace v8App
{
    namespace Threads
    {
        ThreadPoolDelayedQueue::ThreadPoolDelayedQueue(int inNumberOfWorkers, ThreadPriority inPriority) : m_Priority(inPriority)
        {
            // We want to leave one core for the main thread.
            int hardwareThreads = GetHardwareCores();
            if (inNumberOfWorkers < 0)
            {
                inNumberOfWorkers = hardwareThreads;
            }
            m_Queue.SetDelayedJobsReadyDelegate(std::bind(&ThreadPoolDelayedQueue::DelayedJobsReady, this));
            m_NumWorkers = std::max(1, std::min(inNumberOfWorkers, hardwareThreads));
            for (int x = 0; x < m_NumWorkers; x++)
            {
                std::string name;
                if (x == 0)
                {
                    name = Utils::format("DelayedThreadPoolPumper #{}", x);
                    std::unique_ptr<Thread> thread = std::make_unique<ThreadPoolThread>(name, m_Priority, this, true);
                    thread->Start();
                    m_Workers.push_back(std::move(thread));
                }
                else
                {
                    name = Utils::format("DelayedThreadPool #{}", x);
                    std::unique_ptr<Thread> thread = std::make_unique<ThreadPoolThread>(name, m_Priority, this, false);
                    thread->Start();
                    m_Workers.push_back(std::move(thread));
                }
            }
        }

        ThreadPoolDelayedQueue::~ThreadPoolDelayedQueue()
        {
            Terminate();
        }

        bool ThreadPoolDelayedQueue::PostTask(ThreadPoolTaskUniquePtr inTask)
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
        bool ThreadPoolDelayedQueue::PostDelayedTask(double inDelay, ThreadPoolTaskUniquePtr inTask)
        {
            if (m_Exiting)
            {
                return false;
            }
            m_Queue.PushItemDelayed(inDelay, std::move(inTask));
            m_QueueWaiter.notify_one();
            return true;
        }

        void ThreadPoolDelayedQueue::Terminate()
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

        void ThreadPoolDelayedQueue::DelayedJobsReady()
        {
            m_QueueWaiter.notify_all();
        }

        void ThreadPoolDelayedQueue::PumpQueue()
        {
            while (true)
            {
                std::unique_lock<std::mutex> lock(m_QueueLock);
                m_QueueWaiter.wait_for(lock, std::chrono::milliseconds(200), [=]()
                                       { return m_Exiting == true || m_Queue.MayHaveItems(); });
                if (m_Exiting)
                {
                    return;
                }
                if (m_Queue.MayHaveItems())
                {
                    std::optional<ThreadPoolTaskUniquePtr> task = m_Queue.GetNextItem();
                    if (task)
                    {
                        task.value()->Run();
                    }
                }
            }
        }

        void ThreadPoolDelayedQueue::ProcessTasks()
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