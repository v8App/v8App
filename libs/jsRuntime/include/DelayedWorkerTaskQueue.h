// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __DELAYED_TASK_QUEUE_H_
#define __DELAYED_TASK_QUEUE_H_

#include <queue>
#include <mutex>
#include "jsRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        class DelayedWorkerTaskQueue
        {
        public:
            DelayedWorkerTaskQueue();
            virtual ~DelayedWorkerTaskQueue();

            virtual void PostTask(TaskPtr& inTask, double inDelaySeconds);
            TaskPtr PopTask();

            //clears the queue and prevents work from being added
            void Stop();

        protected:
            // so we can override it during testing
            virtual double MonotonicallyIncreasingTime();
            std::mutex m_QueueLock;
            bool m_Stopped = false;

            using DelayedEntry = std::pair<double, TaskPtr>;
            struct DelayedEntryCompare
            {
                bool operator()(const DelayedEntry &left, const DelayedEntry &right)
                {
                    return left.first > right.first;
                }
            };

            std::priority_queue<DelayedEntry, std::vector<DelayedEntry>, DelayedEntryCompare> m_DelayedTasks;
        };
    } // namespace JSRuntime
} // namespace v8App
#endif