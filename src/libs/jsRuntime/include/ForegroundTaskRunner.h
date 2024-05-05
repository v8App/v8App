// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _FOREGROUND_TASK_RUNNER_H_
#define _FOREGROUND_TASK_RUNNER_H_

#include <queue>
#include <mutex>
#include <tuple>

#include "Queues/TThreadSafeQueue.h"

#include "NestableQueue.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        class ForegroundTaskRunner : public v8::TaskRunner
        {
        public:
            class [[nodiscard]] TaskRunScope
            {
            public:
                explicit TaskRunScope(std::shared_ptr<ForegroundTaskRunner> inRunner);
                ~TaskRunScope();

                TaskRunScope(const TaskRunScope &) = delete;
                TaskRunScope &operator=(const TaskRunScope &) = delete;

            private:
                std::shared_ptr<ForegroundTaskRunner> m_Runner;
            };

        public:
            explicit ForegroundTaskRunner();
            ~ForegroundTaskRunner();

            // TaskRunner implementation
            virtual void PostTask(V8TaskUniquePtr inTask) override;
            virtual void PostNonNestableTask(V8TaskUniquePtr inTask) override;
            virtual void PostDelayedTask(V8TaskUniquePtr inTask, double inDelaySeconds) override;
            virtual void PostNonNestableDelayedTask(V8TaskUniquePtr inTask, double inDelaySeconds) override;
            virtual void PostIdleTask(V8IdleTaskUniquePtr) override;
            bool IdleTasksEnabled() override { return true; };
            virtual bool NonNestableTasksEnabled() const override { return true; }
            virtual bool NonNestableDelayedTasksEnabled() const override { return true; }
            // end TaskRunner implementation

            // gets a task from the queue
            V8TaskUniquePtr GetNextTask();
            // get a idle task from the queue
            V8IdleTaskUniquePtr GetNextIdleTask();

            bool MaybeHasTask() { return m_Tasks.MayHaveItems();}
            bool MaybeHasIdleTask() { return m_IdleTasks.MayHaveItems();}
    
            void Terminate();

        protected:
            bool m_Terminated = false;

            NestableQueue m_Tasks;
            Queues::TThreadSafeQueue<V8IdleTaskUniquePtr> m_IdleTasks;
            int m_NestingDepth = 0;
        };
    } // namespace JSRuntime
} // namespace v8App

#endif //_FOREGROUND_TASK_RUNNER_H_