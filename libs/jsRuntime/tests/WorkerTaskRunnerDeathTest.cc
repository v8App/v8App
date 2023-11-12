// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <thread>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WorkerTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {

        class TestIdleDeathTask : public v8::IdleTask
        {
        public:
            void Run(double deadline_in_seconds) {}
        };

        TEST(TaskRunnerTest, IdleTaskDeath)
        {
#ifdef V8APP_DEBUG
            ASSERT_DEATH({
                WorkerTaskRunner runner(4, Threads::ThreadPriority::kDefault);
                V8IdleTaskUniquePtr task = std::make_unique<TestIdleDeathTask>();
                runner.PostIdleTask(std::move(task));
            },
                         "v8App Log \\{");
#endif
        }

    } // namespace JSRuntime
} // namespace v8App