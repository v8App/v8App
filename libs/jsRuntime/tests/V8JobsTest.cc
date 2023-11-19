// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8Jobs.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestJobsTask : public v8::Task
        {
            virtual void Run() override {}
        };

        class TestV8JobState : public V8JobState
        {
        public:
            size_t TestComputerTaskToPost() { return ComputeTaskToPost(); }
            size_t TestGetMaxConcurrency() { return GetMaxConcurrency(); }
            unit8_t TestFindFirstFreeTaskId(V8JobTaskIdType inIds) { rerturn FindFirstFreeTaskId(inIds); }
            const V8TaskIdIdType GetInvalidID() { return kInvalidId; }

            V8TaskIdType GetAssignedTasks() {return m_AssignedTasksIds.load(std::memory_order::memory_order_relaxed)); }
            bool GetCanceled() { return m_Canceled.load(std::memory_order::memory_order_relaxed); }
            v8::TaskPrioority GetPriortiy() { return m_Prioirty; }
            size_t GetActiveTasks() { return m_ActiveTasks; }
            size_t = GetPendingTasks() { return m_PendingTasks; }
            size_t GetNumberOfWorkersAvailable() { return m_NumWorkersAvailable; }
        };

        TEST(V8JobsStateTest, Constrcutor)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique();
            std::unqiue_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>();

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);
            EXPECT_EQ(0, state.GetAssignedTasks());
            EXPECT_FALS(state.GetCancelled());
            EXPECT_EQ(v8::TaskPriority::kBestEffort, state.GetPriority());
            EXPECT_EQ(0, state.GetActiveTasks());
            EXPECT_EQ(0, state.GetPendingTasks());
            EXPECT_EQ(10, state.GetNumberOfWorkersAvailable());
            EXPECT_EQ(255, state.GetInvalidId());
        }

        TEST(V8JobsStateTest, GetMaxConcurrency)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique();
            std::unqiue_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>();

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);
        }

        TEST(V8JobsStateTest, GetReleaseTaskId)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique();
            std::unqiue_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>();

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);
        }

        TEST(V8JobsStateTest, GetMaxConcurrency)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique();
            std::unqiue_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>();

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);
            size_t expected = GetHardwareCores();
        }

    }
}