// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Threads/ThreadPool.h"

namespace v8App
{
    namespace ThreadPool
    {
        std::shared_ptr<ThreadPool> ThreadPool::s_Singleton;

        ThreadPool::ThreadPool(int inNumberOfWorkers)
        {
            int hardwareThreads = std::thread::hardware_concurrency();
            if (inNumberOfWorkers < 0 ) {
                inNumberOfWorkers = hardwareThreads;
            }
            m_NumWorkers = std::max(1, std::min(inNumberOfWorkers, hardwareThreads));
            for (int x = 0; x < m_NumWorkers; x++)
            {
                m_Workers.push_back(std::thread([this]() { this->ProcessTasks(); }));
            }
        }

        ThreadPool::~ThreadPool()
        {
            //if the m_Exiting hasn't been set yet then set it
            if(m_Exiting == false)
            {
                m_Exiting.exchange(true);
            }
            m_QueueWaiter.notify_all();
            for (auto &it : m_Workers)
            {
                it.join();
            }
            m_Workers.clear();
        }

        bool ThreadPool::PostTask(ThreadPoolTaskPtr &&inTask)
        {
            //if we are in the process of exiting then return
            if (m_Exiting)
            {
                return false;
            }

            {
                std::scoped_lock<std::mutex> lock(m_QueueLock);
                m_Tasks.push_back(std::move(inTask));
            }
            m_QueueWaiter.notify_one();
            return true;
        }

        WeakThreadPoolPtr ThreadPool::CreateSingleton(int inNumberOfWorkers)
        {
            //if the pool is already created then skip
            if(s_Singleton.get() != nullptr) 
            {
                return s_Singleton;
            }

            s_Singleton = std::make_shared<ThreadPool>(inNumberOfWorkers);
            return s_Singleton;
        }

        void ThreadPool::ShutdownSingleton()
        {
            //signal we are exiting.
            ThreadPool *ptr = s_Singleton.get();
            if (ptr != nullptr)
            {
                ptr->m_Exiting.exchange(true);
                //clear out the task so no new ones get started.
                std::scoped_lock lock(ptr->m_QueueLock);
                ptr->m_Tasks.clear();
            }

            //now reset the singleton the destructor will clean up the threads once any weak ptrs are finished with it
            s_Singleton.reset();
        }

        void ThreadPool::ProcessTasks()
        {

            while (true)
            {
                std::unique_lock<std::mutex> lock(m_QueueLock);
                m_QueueWaiter.wait(lock, [=]() { 
                    return m_Exiting == true || m_Tasks.empty() == false; 
                    });
                if (m_Exiting)
                {
                    return;
                }

                ThreadPoolTaskPtr task = std::move(m_Tasks.front());
                m_Tasks.pop_front();

                lock.unlock();

                task->Run();
            }
        }
    } // namespace ThreadPool
} // namespace v8App