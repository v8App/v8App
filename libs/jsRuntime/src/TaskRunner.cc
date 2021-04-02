// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"
#include "Time/Time.h"
#include "TaskRunner.h"
#include "V8Platform.h"

namespace v8App
{
    namespace JSRuntime
    {
        TaskRunner::TaskRunScope::TaskRunScope(std::shared_ptr<TaskRunner> inRunner) : m_Runner(inRunner)
        {
            DCHECK_GE(m_Runner->m_NestingDepth, 0);
            m_Runner->m_NestingDepth++;
        }

        TaskRunner::TaskRunScope::~TaskRunScope()
        {
            DCHECK_GT(m_Runner->m_NestingDepth, 0);
            m_Runner->m_NestingDepth--;
        }

        TaskRunner::TaskRunner()
        {
        }

        TaskRunner::~TaskRunner()
        {
        }

        void TaskRunner::PostTask(TaskPtr inTask)
        {
            std::scoped_lock lock(m_QueueLock);
            if(m_Stopped)
            {
                return;
            }
            m_Tasks.push_back(std::make_pair(kNestable, std::move(inTask)));
        }

        void TaskRunner::PostNonNestableTask(TaskPtr inTask)
        {
            std::scoped_lock lock(m_QueueLock);
            if(m_Stopped)
            {
                return;
            }
            m_Tasks.push_back(std::make_pair(kNonNestable, std::move(inTask)));
        }

        void TaskRunner::PostDelayedTask(TaskPtr inTask, double inDelaySeconds)
        {
            DCHECK_GE(inDelaySeconds, 0.0);
            std::scoped_lock lock(m_QueueLock);
            if(m_Stopped)
            {
                return;
            }

            double deadline = MonotonicallyIncreasingTime() + inDelaySeconds;
            m_DelayedTasks.push(std::make_tuple(deadline, kNestable, std::move(inTask)));
        }

        void TaskRunner::PostNonNestableDelayedTask(TaskPtr inTask, double inDelaySeconds)
        {
            DCHECK_GE(inDelaySeconds, 0.0);
            std::scoped_lock lock(m_QueueLock);
            if(m_Stopped)
            {
                return;
            }

            double deadline = MonotonicallyIncreasingTime() + inDelaySeconds;
            m_DelayedTasks.push(std::make_tuple(deadline, kNonNestable, std::move(inTask)));
        }

        void TaskRunner::PostIdleTask(IdleTaskPtr inTask)
        {
            std::scoped_lock lock(m_QueueLock);
            if(m_Stopped)
            {
                return;
            }

            m_IdleTasks.push(std::move(inTask));
        }

        double TaskRunner::MonotonicallyIncreasingTime()
        {
            return Time::MonotonicallyIncreasingTimeSeconds();
        }

        TaskPtr TaskRunner::PopTask()
        {
            std::scoped_lock lock(m_QueueLock);
            if (m_Tasks.empty() || m_Stopped)
            {
                return {};
            }
            auto it = m_Tasks.begin();
            for (; it != m_Tasks.end(); it++)
            {
                if (m_NestingDepth == 0 || it->first == kNestable)
                {
                    break;
                }
            };
            if(it == m_Tasks.end())
            {
                return {};
            }
            TaskPtr task = std::move(it->second);
            m_Tasks.erase(it);
            return task;
        }

        IdleTaskPtr TaskRunner::PopIdleTask()
        {
            std::scoped_lock lock(m_QueueLock);
            if (m_IdleTasks.empty() || m_Stopped)
            {
                return {};
            }

            IdleTaskPtr task = std::move(m_IdleTasks.front());
            m_IdleTasks.pop();

            return task;
        }

        void TaskRunner::ProcessDelayedTasks()
        {
            std::scoped_lock lock(m_QueueLock);

            if (m_DelayedTasks.empty() || m_Stopped)
            {
                return;
            }

            double now = MonotonicallyIncreasingTime();

            //move the tasks that are now runnabel to the taks queue
            while (std::get<0>(m_DelayedTasks.top()) <= now)
            {
                //we need a non const version of the entry for the unqiue_ptr to move
                DelayedEntry& entry = const_cast<DelayedEntry&>(m_DelayedTasks.top());
                //move the nestability and unique_ptr to the task queue
                m_Tasks.push_back(std::make_pair(std::get<1>(entry), std::move(std::get<2>(entry))));
                //now that the pointer is moved we can pop the top.
                m_DelayedTasks.pop();
                //if we are now empty then return;
                if (m_DelayedTasks.empty())
                {
                    return;
                }
            }
        }

        void TaskRunner::Stop(){
            std::scoped_lock lock(m_QueueLock);

            m_Stopped = true;

            m_Tasks.clear();
            while(m_IdleTasks.empty() == false) 
            {
                m_IdleTasks.pop();
            }
            while(m_DelayedTasks.empty() == false)
            {
                m_DelayedTasks.pop();
            }
        }
    } // namespace JSRuntime
} // namespace v8App