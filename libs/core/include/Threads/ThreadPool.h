// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _THREAD_POOL_
#define _THREAD_POOL_

#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <future>

namespace v8App
{
    namespace ThreadPool
    {
        //Base class for all thread tasks
        class ThreadPoolTask
        {
        public:
            ThreadPoolTask() = default;
            virtual ~ThreadPoolTask() = default;

            virtual void Run() = 0;
        };

        //Class that can take a callable and run it. Allows passing lambdas, std::packaged_tasks
        class CallableThreadTask : public ThreadPoolTask
        {
        public:
            struct ImplBase
            {
                inline virtual ~ImplBase() = default;
                virtual void Run() = 0;
            };

            template <typename Callable>
            struct Impl : public ImplBase
            {
                Callable m_Func;
                Impl(Callable &&inFunc) : m_Func(std::forward<Callable>(inFunc))
                {
                }
                void Run() { m_Func(); }
            };

            CallableThreadTask() = delete;

            template <typename Callable>
            explicit CallableThreadTask(Callable inLambda)
            {
                m_Func = std::make_unique<Impl<Callable>>(std::forward<Callable>(inLambda));
            }

            template <typename Callable, typename... Args>
            CallableThreadTask(Callable &&inFunc, Args &&... args)
            {
                m_Func = std::make_unique<Impl<Callable>>(std::forward<Callable>(std::bind(inFunc, args...)));
            }

            void Run() override
            {
                m_Func->Run();
            }

        private:
            std::unique_ptr<ImplBase> m_Func;
        };

        using ThreadPoolTaskPtr = std::unique_ptr<ThreadPoolTask>;
        using WeakThreadPoolPtr = std::weak_ptr<class ThreadPool>;
        using SharedThreadPoolPtr = std::shared_ptr<class ThreadPool>;

        class ThreadPool
        {
        public:
            ThreadPool(int inNumberOfWorkers = -1);
            ~ThreadPool();

            //Add a task to the worker queue
            bool PostTask(ThreadPoolTaskPtr &&inTask);

            //gets the number of worker threads the pool has available
            int GetNumberOfWorkers() { return m_NumWorkers; }

            // pool id is passed when cheking in case a new pool was created after the task was run.
            bool IsExiting() { return m_Exiting; }

            //gets the singleton ThreadPool
            static WeakThreadPoolPtr Get() { return s_Singleton; }
            //creates the thread pool singleton
            static WeakThreadPoolPtr CreateSingleton(int inNumberOfWorkers);
            //shuts down the singleton thread pool.
            static void ShutdownSingleton();

        protected:
            //Hnadles removing a task form the queue and running it.
            void ProcessTasks();

            int m_NumWorkers = 0;
            std::atomic_bool m_Exiting{false};
            std::mutex m_QueueLock;
            std::condition_variable m_QueueWaiter;
            std::deque<ThreadPoolTaskPtr> m_Tasks;
            std::vector<std::thread> m_Workers;

            static SharedThreadPoolPtr s_Singleton;
        };
    } // namespace ThreadPool
} // namespace v8App

#endif