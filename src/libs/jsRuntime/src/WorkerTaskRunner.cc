// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "WorkerTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        WorkerTaskRunner::WorkerTaskRunner(int inNUmberOfWorkers, Threads::ThreadPriority inPriority) : m_Tasks(inNUmberOfWorkers, inPriority)
        {
        }

        WorkerTaskRunner::~WorkerTaskRunner()
        {
            Terminate();
        }

        void WorkerTaskRunner::PostTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation& inLocation)
        {
            Threads::ThreadPoolTaskUniquePtr task = std::make_unique<Threads::CallableThreadTask>([task = std::move(inTask)]{task->Run();});
            m_Tasks.PostTask(std::move(task));
        }

        void WorkerTaskRunner::PostNonNestableTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation& inLocation)
        {
            Threads::ThreadPoolTaskUniquePtr task = std::make_unique<Threads::CallableThreadTask>([task = std::move(inTask)]{task->Run();});
            m_Tasks.PostTask(std::move(task));
        }

        void WorkerTaskRunner::PostDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation& inLocation)
        {
           Threads::ThreadPoolTaskUniquePtr task = std::make_unique<Threads::CallableThreadTask>([task = std::move(inTask)]{task->Run();});
            m_Tasks.PostDelayedTask(inDelaySeconds, std::move(task));
         }

        void WorkerTaskRunner::PostNonNestableDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation& inLocation)
        {
           Threads::ThreadPoolTaskUniquePtr task = std::make_unique<Threads::CallableThreadTask>([task = std::move(inTask)]{task->Run();});
            m_Tasks.PostDelayedTask(inDelaySeconds, std::move(task));
         }

        void WorkerTaskRunner::PostIdleTaskImpl(V8IdleTaskUniquePtr inTask, const V8SourceLocation& inLocation)
        {
            UNIMPLEMENTED();
        }

        void WorkerTaskRunner::Terminate()
        {
            if (m_Terminated)
            {
                return;
            }
            m_Tasks.Terminate();
            m_Terminated = true;
        }
    } // namespace JSRuntime
} // namespace v8App