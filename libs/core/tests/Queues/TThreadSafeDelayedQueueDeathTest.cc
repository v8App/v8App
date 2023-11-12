// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Queues/TThreadSafeDelayedQueue.h"

namespace v8App
{
    namespace Queues
    {
        class DelayedWorkerDeathTestTask
        {
            void Run()
            {
            }
        };

        TEST(TThreadSafeDelayedQueue, PushItemDelayed)
        {
            // Only applicable in debug builds
#ifdef V8APP_DEBUG
            ASSERT_DEATH({
                TThreadSafeDelayedQueue<std::unique_ptr<DelayedWorkerDeathTestTask>> queue;
                std::unique_ptr<DelayedWorkerDeathTestTask> task = std::make_unique<DelayedWorkerDeathTestTask>();
                queue.PushItemDelayed(-1.0, std::move(task));
                std::exit(0);
            },
                         "v8App Log \\{");

            EXPECT_EXIT({
                TThreadSafeDelayedQueue<std::unique_ptr<DelayedWorkerDeathTestTask>> queue;
                std::unique_ptr<DelayedWorkerDeathTestTask> task = std::make_unique<DelayedWorkerDeathTestTask>();
                queue.PushItemDelayed(1.0, std::move(task));
                 std::exit(0);
           },
                        ::testing::ExitedWithCode(0), "");
#endif
        }
    } // namespace Queues
} // namespace v8App