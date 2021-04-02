// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _TASK_RUNNER_H_
#define _TASK_RUNNER_H_

#include <queue>
#include <mutex>
#include <tuple>
#include "v8-platform.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TaskRunner : public v8::TaskRunner
        {
        public:
            explicit TaskRunner();
            ~TaskRunner();

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

            //TaskRunner implementation
            virtual void PostTask(TaskPtr inTask) override;
            virtual void PostNonNestableTask(TaskPtr inTask) override;
            virtual void PostDelayedTask(TaskPtr inTask, double inDelaySeconds) override;
            virtual void PostNonNestableDelayedTask(TaskPtr inTask, double inDelaySeconds) override;
            virtual void PostIdleTask(IdleTaskPtr) override;
            bool IdleTasksEnabled() override { return true; };
            virtual bool NonNestableTasksEnabled() const override { return true; }
            virtual bool NonNestableDelayedTasksEnabled() const override { return true; }

            //gets a task from the queue
            TaskPtr PopTask();
            //get a idle task from the queue
            IdleTaskPtr PopIdleTask();
            //moves delayed tasks that are now ready to the task queue for processing.
            void ProcessDelayedTasks();

            void Stop();

        protected:
            //so we can override it during testing.
            virtual double MonotonicallyIncreasingTime();

            std::mutex m_QueueLock;
            bool m_Stopped = false;

            enum Nestability
            {
                kNestable,
                kNonNestable
            };
            using TaskQueueEntry = std::pair<Nestability, TaskPtr>;
            std::deque<TaskQueueEntry> m_Tasks;
            std::queue<std::unique_ptr<v8::IdleTask>> m_IdleTasks;
            int m_NestingDepth = 0;

            using DelayedEntry = std::tuple<double, Nestability, TaskPtr>;
            struct DelayedEntryCompare
            {
                bool operator()(const DelayedEntry &left, const DelayedEntry &right)
                {
                    return std::get<0>(left) > std::get<0>(right);
                }
            };

            std::priority_queue<DelayedEntry, std::vector<DelayedEntry>, DelayedEntryCompare> m_DelayedTasks;
        };
    } // namespace JSRuntime
} // namespace v8App

#endif