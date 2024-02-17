// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _THREAD_POOL_TASKS_H__
#define _THREAD_POOL_TASKS_H__

#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <future>

#include "Queues/TThreadSafeQueue.h"

namespace v8App
{
    namespace Threads
    {
        // Base class for all thread tasks
        class IThreadPoolTask
        {
        public:
            virtual ~IThreadPoolTask() = default;
            virtual void Run() = 0;
        };

        // Class that can take a callable and run it. Allows passing lambdas, std::packaged_tasks
        class CallableThreadTask : public IThreadPoolTask
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
                Impl(Callable &&inFunc) : m_Func(std::move(inFunc))
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
            CallableThreadTask(Callable &&inFunc, Args &&...args)
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

        using ThreadPoolTaskUniquePtr = std::unique_ptr<IThreadPoolTask>;
    } // namespace Threads
} // namespace v8App

#endif //_THREAD_POOL_TASKS_H__