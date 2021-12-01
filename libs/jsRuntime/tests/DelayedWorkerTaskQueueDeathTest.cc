// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "DelayedWorkerTaskQueue.h"

namespace v8App
{
    namespace JSRuntime
    {
        class DelayedWorkerDeathTestTask : public v8::Task
        {
            void Run() override
            {
            }
        };
        
        TEST(DelayedWorkerTaskQueueDeathTest, PostTest)
        {
            //Only applicable in debug builds
#ifdef V8APP_DEBUG
            ASSERT_DEATH({
                DelayedWorkerTaskQueue queue;
                TaskPtr task = std::make_unique<DelayedWorkerDeathTestTask>();
                queue.PostTask(task, -1.0);
            },
                         "v8App Log {");
#endif
        }
    } // namespace JSRuntime
} // namespace v8App