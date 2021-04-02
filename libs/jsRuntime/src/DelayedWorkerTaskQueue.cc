
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "DelayedWorkerTaskQueue.h"
#include "Logging/LogMacros.h"
#include "Time/Time.h"

namespace v8App
{
    namespace JSRuntime
    {
        DelayedWorkerTaskQueue::DelayedWorkerTaskQueue() 
        {
        }

        DelayedWorkerTaskQueue::~DelayedWorkerTaskQueue()
        {
        }

        void DelayedWorkerTaskQueue::PostTask(TaskPtr& inTask, double inDelaySeconds)
        {

            DCHECK_GE(inDelaySeconds, 0.0);
            std::scoped_lock lock(m_QueueLock);
            if(m_Stopped)
            {
                return;
            }

            double deadline = MonotonicallyIncreasingTime() + inDelaySeconds;
            m_DelayedTasks.push(std::make_pair(deadline, std::move(inTask)));
        }

        TaskPtr DelayedWorkerTaskQueue::PopTask()
        {
            std::scoped_lock lock(m_QueueLock);

            if (m_DelayedTasks.empty() || m_Stopped)
            {
                return {};
            }

            double now = MonotonicallyIncreasingTime();

            //move the tasks that are now runnabel to the taks queue
            //we need a non const version of the entry for the unqiue_ptr to move
            DelayedEntry &entry = const_cast<DelayedEntry &>(m_DelayedTasks.top());
            if (entry.first > now)
            {
                return {};
            }
            //move the unique_ptr to a temp var
            TaskPtr task = std::move(entry.second);
            m_DelayedTasks.pop();

            return task;
        }

        double DelayedWorkerTaskQueue::MonotonicallyIncreasingTime()
        {
            return Time::MonotonicallyIncreasingTimeSeconds();
        }

        void DelayedWorkerTaskQueue::Stop()
        {
            std::scoped_lock lock(m_QueueLock);

            m_Stopped = true;
            while (m_DelayedTasks.empty() == false)
            {
                m_DelayedTasks.pop();
            }
        }
    } // namespace JSRuntime
} // namespace v8App
