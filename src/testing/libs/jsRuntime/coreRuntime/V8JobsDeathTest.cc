// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Platform.h"
#include "V8Jobs.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestJobsTaskDeath : public v8::JobTask
        {
        public:
            TestJobsTaskDeath(size_t inConcurrency) : m_Concurrency(inConcurrency) {}
            virtual void Run(v8::JobDelegate *delegate) override {}
            virtual size_t GetMaxConcurrency(size_t worker_count) const override { return m_Concurrency; }
            size_t m_Concurrency;
        };


        class TestV8JobStateDeath : public V8JobState
        {
        public:
            TestV8JobStateDeath(v8::Platform *inPlatform, V8JobTaskUniquePtr inTask, v8::TaskPriority inPriority, size_t inNumWorkers)
                : V8JobState(inPlatform, std::move(inTask), inPriority, inNumWorkers) {}

            void SetCancelled(bool value) { m_Canceled.store(value, std::memory_order_relaxed); }
        };

        TEST(V8JobDelegateDeathTest, ShouldYield)
        {
#ifdef V8APP_DEBUG
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
                std::unique_ptr<TestJobsTaskDeath> task = std::make_unique<TestJobsTaskDeath>(5);

                TestV8JobStateDeath state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);
                std::unique_ptr<V8JobState::V8JobDelegate> delegate = std::make_unique<V8JobState::V8JobDelegate>(&state, false);

                state.SetCancelled(true);
                EXPECT_TRUE(delegate->ShouldYield());
                delegate->ShouldYield();
                std::exit(0);
            },
                         "v8App Log \\{");
#endif
        }
    }
}