// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "ForegroundTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        ForegroundTaskRunner::TaskRunScope::TaskRunScope(std::shared_ptr<ForegroundTaskRunner> inRunner) : m_Runner(inRunner)
        {
            DCHECK_GE(m_Runner->m_NestingDepth, 0);
            m_Runner->m_NestingDepth++;
        }

        ForegroundTaskRunner::TaskRunScope::~TaskRunScope()
        {
            DCHECK_GT(m_Runner->m_NestingDepth, 0);
            m_Runner->m_NestingDepth--;
        }

        ForegroundTaskRunner::ForegroundTaskRunner() : m_Tasks()
        {
        }

        ForegroundTaskRunner::~ForegroundTaskRunner()
        {
            Terminate();
        }

        void ForegroundTaskRunner::PostTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation& inLocation)
        {
            m_Tasks.PushItem(std::move(inTask));
        }

        void ForegroundTaskRunner::PostNonNestableTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation& inLocation)
        {
            m_Tasks.PushNonNestableItem(std::move(inTask));
        }

        void ForegroundTaskRunner::PostDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation& inLocation)
        {
            m_Tasks.PushItemDelayed(inDelaySeconds, std::move(inTask));
        }

        void ForegroundTaskRunner::PostNonNestableDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation& inLocation)
        {
            m_Tasks.PushNonNestableItemDelayed(inDelaySeconds, std::move(inTask));
        }

        void ForegroundTaskRunner::PostIdleTaskImpl(V8IdleTaskUniquePtr inTask, const V8SourceLocation& inLocation)
        {
            m_IdleTasks.PushItem(std::move(inTask));
        }

        V8TaskUniquePtr ForegroundTaskRunner::GetNextTask()
        {
            std::optional<V8TaskUniquePtr> task = m_Tasks.GetNextItem(m_NestingDepth);
            if(task.has_value())
            {
                return std::move(task.value());
            }
            return V8TaskUniquePtr();
        }

        V8IdleTaskUniquePtr ForegroundTaskRunner::GetNextIdleTask()
        {
            std::optional<V8IdleTaskUniquePtr> task = m_IdleTasks.GetNextItem();
            if(task.has_value())
            {
                return std::move(task.value());
            }
            return V8IdleTaskUniquePtr();
        }


        void ForegroundTaskRunner::Terminate(){
            if(m_Terminated)
            {
                return;
            }
            m_Tasks.Terminate();
            m_IdleTasks.Terminate();
            m_Terminated = true;
        }
    } // namespace JSRuntime
} // namespace v8App