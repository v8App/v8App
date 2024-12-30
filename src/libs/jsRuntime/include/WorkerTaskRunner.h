// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
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

            /**
             * Termintate the task
             */
            void Terminate();

            /**
             * Pause the task. Probably can be removed think was added
             * as part of snapshotting work but not needed
             */
            bool SetPaused(bool inPause) { return m_Tasks.SetPaused(inPause); }

            // TaskRunner implementation
        public:
            bool IdleTasksEnabled() override { return false; };
            virtual bool NonNestableTasksEnabled() const override { return false; }
            virtual bool NonNestableDelayedTasksEnabled() const override { return false; }

        protected:
            virtual void PostTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation &inLocation) override;
            virtual void PostNonNestableTaskImpl(V8TaskUniquePtr task, const V8SourceLocation &inLocation) override;
            virtual void PostDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation &inLocation) override;
            virtual void PostNonNestableDelayedTaskImpl(V8TaskUniquePtr task, double delay_in_seconds,
                                                        const V8SourceLocation &inLocation) override;
            virtual void PostIdleTaskImpl(V8IdleTaskUniquePtr, const V8SourceLocation &inLocation) override;
            // end TaskRunner implementation

        protected:
            bool m_Terminated = false;
            Threads::ThreadPoolDelayedQueue m_Tasks;
        };
    } // namespace JSRuntime
} // namespace v8App

#endif //_WORKER_TASK_RUNNER_H_