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

            // gets a task from the queue
            V8TaskUniquePtr GetNextTask();
            // get a idle task from the queue
            V8IdleTaskUniquePtr GetNextIdleTask();

            bool MaybeHasTask() { return m_Tasks.MayHaveItems(); }
            bool MaybeHasIdleTask() { return m_IdleTasks.MayHaveItems(); }

            void Terminate();

            // TaskRunner implementation
        public:
            bool IdleTasksEnabled() override { return true; };
            virtual bool NonNestableTasksEnabled() const override { return true; }
            virtual bool NonNestableDelayedTasksEnabled() const override { return true; }

        protected:
            virtual void PostTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation &inLocation) override;
            virtual void PostNonNestableTaskImpl(V8TaskUniquePtr inTask, const V8SourceLocation &inLocation) override;
            virtual void PostDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation &inLocation) override;
            virtual void PostNonNestableDelayedTaskImpl(V8TaskUniquePtr inTask, double inDelaySeconds, const V8SourceLocation &inLocation) override;
            virtual void PostIdleTaskImpl(V8IdleTaskUniquePtr, const V8SourceLocation &inLocation) override;
            // end TaskRunner implementation

        protected:
            bool m_Terminated = false;

            NestableQueue m_Tasks;
            Queues::TThreadSafeQueue<V8IdleTaskUniquePtr> m_IdleTasks;
            int m_NestingDepth = 0;
        };
    } // namespace JSRuntime
} // namespace v8App

#endif //_FOREGROUND_TASK_RUNNER_H_