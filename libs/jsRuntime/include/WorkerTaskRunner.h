// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _WORKER_TASK_RUNNER_H_
#define _WORKER_TASK_RUNNER_H_

#include <queue>
#include <mutex>
#include <tuple>

#include "Threads/ThreadPoolDelayedQueue.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        class WorkerTaskRunner : public v8::TaskRunner
        {
        public:
            explicit WorkerTaskRunner(int iNumberofWorkers, Threads::ThreadPriority inPriority);
            ~WorkerTaskRunner();

            class TaskRunScope
            {
            public:
                explicit TaskRunScope(std::shared_ptr<TaskRunner> inRunner);
                ~TaskRunScope();

                TaskRunScope(const TaskRunScope &) = delete;
                TaskRunScope &operator=(const TaskRunScope &) = delete;

            private:
                std::shared_ptr<TaskRunner> m_Runner;
            };

            // TaskRunner implementation
            virtual void PostTask(V8TaskUniquePtr inTask) override;
            virtual void PostDelayedTask(V8TaskUniquePtr inTask, double inDelaySeconds) override;
            virtual void PostIdleTask(V8IdleTaskUniquePtr) override;
            bool IdleTasksEnabled() override { return false; };
            virtual bool NonNestableTasksEnabled() const override { return false; }
            virtual bool NonNestableDelayedTasksEnabled() const override { return false; }
            // end TaskRunner implementation

            void Terminate();

        protected:
             bool m_Terminated = false;
            Threads::ThreadPoolDelayedQueue m_Tasks;
        };
    } // namespace JSRuntime
} // namespace v8App

#endif //_WORKER_TASK_RUNNER_H_