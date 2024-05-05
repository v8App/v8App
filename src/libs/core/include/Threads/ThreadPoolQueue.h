// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _THREAD_POOL_QUEUE_H__
#define _THREAD_POOL_QUEUE_H__

#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include <future>

#include "Queues/TThreadSafeQueue.h"
#include "Threads/Threads.h"
#include "Threads/ThreadPoolTasks.h"

namespace v8App
{
    namespace Threads
    {
        /**
         * ThreadPoolQueue Implements a queue that has a thread pool that runs all the tasks posted to it
         */
        class ThreadPoolQueue
        {
        public:
            ThreadPoolQueue(int inNumberOfWorkers = -1, ThreadPriority inPriority = ThreadPriority::kBestEffort);
            ~ThreadPoolQueue();

            // Add a task to the worker queue
            bool PostTask(ThreadPoolTaskUniquePtr inTask);

            // gets the number of worker threads the pool has available
            int GetNumberOfWorkers() { return m_NumWorkers; }

            // pool id is passed when cheking in case a new pool was created after the task was run.
            bool IsExiting() { return m_Exiting; }
            ThreadPriority GetPriority() const { return m_Priority; }

            void Terminate();

        protected:
            class ThreadPoolThread : public Thread
            {
            public:
                ThreadPoolThread(std::string inName, ThreadPriority inPriority, ThreadPoolQueue *inPool) : Thread(inName, inPriority), m_Pool(inPool) {}

            protected:
                virtual void RunImpl() override
                {
                    m_Pool->ProcessTasks();
                }

            protected:
                ThreadPoolQueue *m_Pool;
            };

            // Hnadles removing a task form the queue and running it.
            void ProcessTasks();

            int m_NumWorkers = 0;
            std::mutex m_QueueLock;
            std::atomic_bool m_Exiting{false};
            std::condition_variable m_QueueWaiter;
            Queues::TThreadSafeQueue<ThreadPoolTaskUniquePtr> m_Queue;
            std::vector<std::unique_ptr<Thread>> m_Workers;
            ThreadPriority m_Priority;
        };
    } // namespace Threads
} // namespace v8App

#endif //_THREAD_POOL_QUEUE_H__