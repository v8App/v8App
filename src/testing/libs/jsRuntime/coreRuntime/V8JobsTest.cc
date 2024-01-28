// Copyright 2020 The v8App Authors. All rights reserved.
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
        static std::queue<int> testJobTaskSleepTimes;
        static std::queue<int> testJobTaskValues;
        std::mutex testJobTaskQueuesLock;

        class TestJobsTask : public v8::JobTask
        {
        public:
            TestJobsTask(size_t inConcurrency) : m_Concurrency(inConcurrency) {}
            virtual void Run(v8::JobDelegate *delegate) override {}
            virtual size_t GetMaxConcurrency(size_t worker_count) const override { return m_Concurrency; }
            size_t m_Concurrency;
        };

        class TestJobsPostTask : public v8::JobTask
        {
        public:
            explicit TestJobsPostTask(size_t *inPtr) : m_Ptr(inPtr) {}
            virtual void Run(v8::JobDelegate *delegate) override
            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                // in case we didn't set enough values and additional tasks get queued up
                // by the worker task
                if (testJobTaskSleepTimes.size() == 0)
                {
                    return;
                }
                int sleep = testJobTaskSleepTimes.front();
                testJobTaskSleepTimes.pop();
                int value = testJobTaskValues.front();
                testJobTaskValues.pop();

                std::this_thread::sleep_for(std::chrono::seconds(sleep));
                *m_Ptr = value;
                if(m_Concurrency > -1)
                {
                    m_Concurrency--;
                }
            }
            virtual size_t GetMaxConcurrency(size_t worker_count) const override { 
                if(m_Concurrency != -1)
                {
                    return m_Concurrency;
                }
                return Threads::GetHardwareCores(); 
                }

            size_t *m_Ptr;
            int m_Concurrency =  -1;
        };

        class TestV8JobState : public V8JobState
        {
        public:
            TestV8JobState(v8::Platform *inPlatform, V8JobTaskUniquePtr inTask, v8::TaskPriority inPriority, size_t inNumWorkers)
                : V8JobState(inPlatform, std::move(inTask), inPriority, inNumWorkers) {}

            size_t TestComputeTaskToPost(size_t inMaxConcurrency) { return ComputeTaskToPost(inMaxConcurrency); }
            size_t TestGetMaxConcurrency(size_t inWorkerCountr) { return GetMaxConcurrency(inWorkerCountr); }
            const V8JobTaskIdType GetInvalidId() { return kInvalidJobId; }
            void TestPostonWorkerThread(size_t inNumToPost, v8::TaskPriority inPriority) { PostonWorkerThread(inNumToPost, inPriority); }

            V8JobTaskIdType GetAssignedTasks() { return m_AssignedTaskIds.load(std::memory_order::relaxed); }
            bool GetCancelled() { return m_Canceled.load(std::memory_order::relaxed); }
            v8::TaskPriority GetPriority() { return m_Priority; }
            size_t GetActiveTasks() { return m_ActiveTasks; }
            size_t GetPendingTasks() { return m_PendingTasks; }
            size_t GetNumberOfWorkersAvailable() { return m_NumWorkersAvailable; }

            void SetActiveTasks(size_t num) { m_ActiveTasks = num; }
            void SetPendingTasks(size_t num) { m_PendingTasks = num; }
            void SetCancelled(bool value) { m_Canceled.store(value, std::memory_order::relaxed); }
        };

        class TestJobsPostTaskCancel : public v8::JobTask
        {
        public:
            explicit TestJobsPostTaskCancel(size_t *inPtr) : m_Ptr(inPtr) {}
            virtual void Run(v8::JobDelegate *delegate) override
            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                // in case we didn't set enough values and additional tasks get queued up
                // by the worker task
                if (testJobTaskSleepTimes.size() == 0)
                {
                    return;
                }
                int sleep = testJobTaskSleepTimes.front();
                testJobTaskSleepTimes.pop();
                int value = testJobTaskValues.front();
                testJobTaskValues.pop();

                std::this_thread::sleep_for(std::chrono::seconds(sleep));
                *m_Ptr = value;
                // Since we run this from the main thread we need to be able to cancel the run.
                // We want 2 runs and then for it to quit.
                m_State->SetCancelled(true);
            }
            virtual size_t GetMaxConcurrency(size_t worker_count) const override { return Threads::GetHardwareCores(); }

            size_t *m_Ptr;
            TestV8JobState *m_State;
        };

        class TestV8JobDelegate : public V8JobState::V8JobDelegate
        {
        public:
            explicit TestV8JobDelegate(V8JobState *inState, bool isJoiningThread = false) : V8JobState::V8JobDelegate(inState, isJoiningThread) {}
            bool GetYielded() { return m_Yielded; }
            V8JobState *GetState() { return m_JobState; }
        };

        class TestJobTaskWorker : public V8JobTaskWorker
        {
        public:
            TestJobTaskWorker(std::weak_ptr<V8JobState> inState, v8::JobTask *inTask) : V8JobTaskWorker(inState, inTask) {}

            V8JobState *GeState() { return m_State.lock().get(); }
            v8::JobTask *GetTask() { return m_Task; }
        };

        class TestJobHandle : public V8JobHandle
        {
        public:
            TestJobHandle(std::shared_ptr<V8JobState> inState) : V8JobHandle(inState) {}

            V8JobState *GetState() { return m_State.get(); }
            void SetState(std::shared_ptr<V8JobState> inState) { m_State = inState; }
            void ClearState() { m_State = nullptr; }
        };

        TEST(V8JobsStateTest, Constrcutor)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(5);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);
            EXPECT_EQ(0, state.GetAssignedTasks());
            EXPECT_FALSE(state.GetCancelled());
            EXPECT_EQ(v8::TaskPriority::kBestEffort, state.GetPriority());
            EXPECT_EQ(0, state.GetActiveTasks());
            EXPECT_EQ(0, state.GetPendingTasks());
            EXPECT_EQ(10, state.GetNumberOfWorkersAvailable());
            EXPECT_EQ(255, state.GetInvalidId());
        }

        TEST(V8JobsStateTest, GetMaxConcurrency)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(5);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);

            EXPECT_EQ(5, state.TestGetMaxConcurrency(10));
            EXPECT_EQ(5, state.TestGetMaxConcurrency(2));
        }

        TEST(V8JobsStateTest, GetReleaseTaskId)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(5);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);

            EXPECT_EQ(0, state.GetAssignedTasks());
#ifdef PLATFORM_64
            EXPECT_EQ(64, V8JobTaskBits);
#else
            EXPECT_EQ(32, V8JobTaskBits);
#endif
            uint8_t i = 0;
            for (; i < V8JobTaskBits; i++)
            {
                EXPECT_EQ(i, state.AcquireTaskId());
            }
            EXPECT_EQ(state.GetInvalidId(), state.AcquireTaskId());
            state.ReleaseTaskID(3);
            state.ReleaseTaskID(10);
            EXPECT_EQ(3, state.AcquireTaskId());
            EXPECT_EQ(10, state.AcquireTaskId());
        }

        TEST(V8JobsStateTest, ComputeTestToPost)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(5);
            TestJobsTask *taskPtr = task.get();

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 10);
            EXPECT_EQ(5, state.TestComputeTaskToPost(5));
            EXPECT_EQ(5, state.GetPendingTasks());
            EXPECT_EQ(0, state.TestComputeTaskToPost(5));

            state.SetPendingTasks(0);
            state.SetActiveTasks(5);
            EXPECT_EQ(0, state.TestComputeTaskToPost(5));

            state.SetActiveTasks(5);
            state.SetActiveTasks(5);
            taskPtr->m_Concurrency = 2;
            EXPECT_EQ(0, state.TestComputeTaskToPost(5));

            state.SetActiveTasks(1);
            state.SetPendingTasks(1);
            taskPtr->m_Concurrency = 5;
            EXPECT_EQ(3, state.TestComputeTaskToPost(5));
            EXPECT_EQ(4, state.GetPendingTasks());
            // set it back to 0 or we'll get an assert
            state.SetActiveTasks(0);
        }

        TEST(V8JobsStateTest, PostOnWorkerThread)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }

            state->SetCancelled(true);
            state->SetPendingTasks(1);
            state->TestPostonWorkerThread(1, v8::TaskPriority::kBestEffort);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_EQ(0, testInt);

            state->SetCancelled(false);
            state->SetPendingTasks(1);
            state->TestPostonWorkerThread(0, v8::TaskPriority::kBestEffort);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            EXPECT_EQ(0, testInt);

            state->TestPostonWorkerThread(1, v8::TaskPriority::kBestEffort);
            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_EQ(5, testInt);
            // Have to cancel so that any additional jobs posted by the worker thread stop
            state->CancelAndWait();
        }

        TEST(V8JobsStateTest, NotifyConcurrencyIncrease)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }

            state->SetCancelled(true);
            state->NotifyConcurrencyIncrease();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_EQ(0, testInt);

            state->SetCancelled(false);
            state->NotifyConcurrencyIncrease();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_EQ(5, testInt);
            // Have to cancel so that any additional jobs posted by the worker thread stop
            state->CancelAndWait();
        }

        TEST(V8JobsStateTest, UpdatePriority)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(5);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);

            EXPECT_EQ(v8::TaskPriority::kBestEffort, state.GetPriority());
            state.UpdatePriority(v8::TaskPriority::kUserBlocking);
            EXPECT_EQ(v8::TaskPriority::kUserBlocking, state.GetPriority());
        }

        TEST(V8JobsStateTest, IsActive)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(0);
            TestJobsTask *taskPtr = task.get();

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);

            EXPECT_FALSE(state.IsActive());
            state.SetActiveTasks(5);
            EXPECT_TRUE(state.IsActive());
            state.SetActiveTasks(0);
            taskPtr->m_Concurrency = 4;
            EXPECT_TRUE(state.IsActive());
        }

        TEST(V8JobsStateTest, CanDidRunFirstTask)
        {
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsTask> task = std::make_unique<TestJobsTask>(5);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);

            state->SetCancelled(true);
            state->SetPendingTasks(3);
            EXPECT_FALSE(state->CanRunFirstTask());

            state->SetCancelled(false);
            state->SetActiveTasks(7);
            EXPECT_FALSE(state->CanRunFirstTask());

            state->SetActiveTasks(0);
            EXPECT_TRUE(state->CanRunFirstTask());
            EXPECT_EQ(1, state->GetActiveTasks());

            state->SetCancelled(true);
            state->SetActiveTasks(3);
            EXPECT_FALSE(state->DidRunFirstTask());
            EXPECT_EQ(2, state->GetActiveTasks());
            state->SetActiveTasks(1);
            EXPECT_FALSE(state->DidRunFirstTask());
            EXPECT_EQ(0, state->GetActiveTasks());

            state->SetCancelled(false);
            state->SetActiveTasks(0);
            EXPECT_TRUE(state->DidRunFirstTask());
            // Have to cancel so that any additional jobs posted by the worker thread stop
            state->CancelAndWait();
        }

        TEST(V8JobsStateTest, Join)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);
            task->m_Concurrency = 2;

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
                testJobTaskValues.push(4);
            }
            auto start = std::chrono::high_resolution_clock::now();
            state->Join();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(4, elapsed);
            EXPECT_GE(4, testInt);
        }

        TEST(V8JobsStateTest, CancelAndWait)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }
            auto start = std::chrono::high_resolution_clock::now();
            state.CancelAndWait();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(2, elapsed);
            EXPECT_GE(5, testInt);
        }

        TEST(V8JobsStateTest, CancelAndDetach)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }
            auto start = std::chrono::high_resolution_clock::now();
            state.CancelAndDetach();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_LE(elapsed, 1);

            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_GE(5, testInt);
        }

        TEST(V8JobDelegateTest, Constrcutor)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);
            TestV8JobDelegate delegate(&state, false);

            EXPECT_FALSE(delegate.IsJoiningThread());
            EXPECT_FALSE(delegate.GetYielded());
            EXPECT_EQ(&state, delegate.GetState());
        }

        TEST(V8JobDelegateTest, ShouldYield)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);
            TestV8JobDelegate delegate(&state, false);

            EXPECT_FALSE(delegate.ShouldYield());
            EXPECT_FALSE(delegate.ShouldYield());
            state.SetCancelled(true);
            EXPECT_TRUE(delegate.ShouldYield());
            EXPECT_TRUE(delegate.GetYielded());
        }

        TEST(V8JobDelegateTest, GetTaskId)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);
            TestV8JobDelegate delegate(&state, false);

            EXPECT_EQ(0, delegate.GetTaskId());
            EXPECT_EQ(0, delegate.GetTaskId());
            EXPECT_EQ(1, state.AcquireTaskId());
        }

        TEST(V8JobDelegateTest, IsJoiningThread)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            TestV8JobState state(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 1);
            std::unique_ptr<TestV8JobDelegate> delegate = std::make_unique<TestV8JobDelegate>(&state, false);

            EXPECT_FALSE(delegate->IsJoiningThread());

            delegate = std::make_unique<TestV8JobDelegate>(&state, true);
            EXPECT_TRUE(delegate->IsJoiningThread());
        }

        TEST(V8JobDelegateTest, NotifyConcurrencyIncrease)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestV8JobDelegate delegate(state.get(), false);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }

            delegate.NotifyConcurrencyIncrease();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_EQ(5, testInt);
            state->CancelAndWait();
        }

        TEST(V8JobHandleTest, Constrcutor)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobHandle handle(state);

            EXPECT_EQ(state.get(), handle.GetState());
            EXPECT_TRUE(handle.IsActive());
            EXPECT_TRUE(handle.IsValid());

            TestJobHandle handle2(nullptr);
            EXPECT_FALSE(handle2.IsValid());
            handle.ClearState();
            handle2.ClearState();
        }

        TEST(V8JobHandleTest, UpdatePriority)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobHandle handle(state);

            EXPECT_TRUE(handle.UpdatePriorityEnabled());
            EXPECT_EQ(v8::TaskPriority::kBestEffort, state->GetPriority());

            state->UpdatePriority(v8::TaskPriority::kUserBlocking);
            EXPECT_EQ(v8::TaskPriority::kUserBlocking, state->GetPriority());
            handle.ClearState();
        }

        TEST(V8JobHandleTest, NotifyConcurrencyIncrease)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobHandle handle(state);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }

            handle.NotifyConcurrencyIncrease();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_EQ(5, testInt);
            handle.Cancel();
        }

        TEST(V8JobHandleTest, Join)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);
            task->m_Concurrency = 2;

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobHandle handle(state);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
                testJobTaskValues.push(4);
            }
            auto start = std::chrono::high_resolution_clock::now();
            handle.Join();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(4, elapsed);
            EXPECT_GE(4, testInt);
        }

        TEST(V8JobHandleTest, Cancel)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobHandle handle(state);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }
            auto start = std::chrono::high_resolution_clock::now();
            handle.Cancel();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(2, elapsed);
            EXPECT_GE(5, testInt);
        }

        TEST(V8JobHandleTest, CancelAnDetach)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobHandle handle(state);

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }
            auto start = std::chrono::high_resolution_clock::now();
            handle.CancelAndDetach();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_LE(elapsed, 1);

            std::this_thread::sleep_for(std::chrono::seconds(3));
            EXPECT_GE(5, testInt);
        }

        TEST(V8JobTaskWorker, Constrcutor)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTask> task = std::make_unique<TestJobsPostTask>(&testInt);
            TestJobsPostTask *taskPtr = task.get();

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            TestJobTaskWorker worker(state, taskPtr);

            EXPECT_EQ(state.get(), worker.GeState());
            EXPECT_EQ(taskPtr, worker.GetTask());
        }

        TEST(V8JobTaskWorker, Run)
        {
            size_t testInt = 0;
            std::unique_ptr<V8Platform> platform = std::make_unique<V8Platform>();
            std::unique_ptr<TestJobsPostTaskCancel> task = std::make_unique<TestJobsPostTaskCancel>(&testInt);
            TestJobsPostTaskCancel *taskPtr = task.get();

            std::shared_ptr<TestV8JobState> state = std::make_shared<TestV8JobState>(platform.get(), std::move(task), v8::TaskPriority::kBestEffort, 2);
            std::shared_ptr<TestV8JobState> emptyState;
            taskPtr->m_State = state.get();

            TestJobTaskWorker worker(emptyState, task.get());
            TestJobTaskWorker worker2(state, taskPtr);
            EXPECT_EQ(nullptr, worker.GeState());

            {
                std::lock_guard<std::mutex> lock(testJobTaskQueuesLock);
                testJobTaskSleepTimes.push(2);
                testJobTaskValues.push(5);
            }

            worker.Run();
            EXPECT_EQ(0, testInt);

            state->SetCancelled(true);
            worker2.Run();
            EXPECT_EQ(0, testInt);

            state->SetCancelled(false);
            state->SetPendingTasks(1);
            auto start = std::chrono::high_resolution_clock::now();
            worker2.Run();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_EQ(5, testInt);
            EXPECT_GE(elapsed, 2);
            state->CancelAndWait();
        }

    }

}