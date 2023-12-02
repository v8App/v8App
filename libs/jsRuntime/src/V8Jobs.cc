// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Logging/LogMacros.h"

#include "V8Jobs.h"

namespace v8App
{
    namespace JSRuntime
    {
        V8JobState::V8JobDelegate::V8JobDelegate(V8JobState *inState, bool isJoiningThread) : m_JobState(inState), m_JoingThread(isJoiningThread)
        {
            static_assert(V8JobState::kInvalidJobId >= V8JobTaskBits, "kInvalidJobID is not outside of the assignable task ids");
        }

        V8JobState::V8JobDelegate::~V8JobDelegate()
        {
            if (m_TaskId != V8JobState::kInvalidJobId)
            {
                m_JobState->ReleaseTaskID(m_TaskId);
            }
        }

        bool V8JobState::V8JobDelegate::ShouldYield()
        {
            DCHECK_FALSE(m_Yielded);
            m_Yielded |= m_JobState->m_Canceled.load(std::memory_order::relaxed);
            return m_Yielded;
        }

        void V8JobState::V8JobDelegate::NotifyConcurrencyIncrease()
        {
            m_JobState->NotifyConcurrencyIncrease();
        }

        uint8_t V8JobState::V8JobDelegate::GetTaskId()
        {
            if (m_TaskId == V8JobState::kInvalidJobId)
            {
                m_TaskId = m_JobState->AcquireTaskId();
            }
            return m_TaskId;
        }

        bool V8JobState::V8JobDelegate::IsJoiningThread() const
        {
            return m_JoingThread;
        }

        V8JobState::V8JobState(v8::Platform *inPlatfor, V8JobTaskUniquePtr inTask, v8::TaskPriority inPriority, size_t inNumWorkers)
            : m_Platform(inPlatfor), m_Task(std::move(inTask)), m_Priority(inPriority), m_NumWorkersAvailable(inNumWorkers)
        {
        }

        V8JobState::~V8JobState()
        {
            DCHECK_EQ(0, m_ActiveTasks);
        }

        void V8JobState::NotifyConcurrencyIncrease()
        {
            if (m_Canceled.load(std::memory_order::relaxed))
            {
                return;
            }
            v8::TaskPriority priority;
            {
                std::unique_lock<std::mutex> lock(m_Lock);
                priority = m_Priority;
            }
            PostonWorkerThread(ComputeTaskToPost(GetMaxConcurrency(m_ActiveTasks)), priority);
        }

        uint8_t V8JobState::AcquireTaskId()
        {
            V8JobTaskIdType assignedTaskIds = m_AssignedTaskIds.load(std::memory_order::relaxed);
            V8JobTaskIdType newTaskIds;
            uint8_t taskID;
            do
            {
                taskID = FindFirstFreeTaskId(assignedTaskIds);
                if (taskID == kInvalidJobId)
                {
                    return taskID;
                }
                newTaskIds = m_AssignedTaskIds | (1 << taskID);
            } while (m_AssignedTaskIds.compare_exchange_weak(assignedTaskIds, newTaskIds, std::memory_order::acquire, std::memory_order::relaxed));
            return taskID;
        }

        void V8JobState::ReleaseTaskID(uint8_t inTaskId)
        {
            V8JobTaskIdType previousTaskIDs = m_AssignedTaskIds.fetch_and(~(1 << inTaskId), std::memory_order::release);
            DCHECK_TRUE(previousTaskIDs & (1 << inTaskId));
            (void)previousTaskIDs;
        }

        void V8JobState::Join()
        {
            auto WaitForRunOpportunity = [this]() -> size_t
            {
                std::unique_lock<std::mutex> lock(m_Lock);
                size_t maxConcurrency = GetMaxConcurrency(m_ActiveTasks - 1);
                while (m_ActiveTasks > maxConcurrency && m_ActiveTasks > 1)
                {
                    m_WorkerReleased.wait(lock);
                    maxConcurrency = GetMaxConcurrency(m_ActiveTasks - 1);
                }
                DCHECK_LE(0, maxConcurrency);
                if (maxConcurrency != 0)
                {
                    return maxConcurrency;
                }
                DCHECK_EQ(1, m_ActiveTasks);
                m_ActiveTasks = 0;
                m_Canceled.store(true, std::memory_order::relaxed);
                return 0;
            };

            {
                std::unique_lock<std::mutex> lock(m_Lock);
                m_Priority = v8::TaskPriority::kUserBlocking;
                m_ActiveTasks++;
                m_NumWorkersAvailable++;
            }
            size_t maxConcurrency = WaitForRunOpportunity();
            if (maxConcurrency == 0)
            {
                return;
            }
            PostonWorkerThread(ComputeTaskToPost(maxConcurrency), m_Priority);
            V8JobState::V8JobDelegate delegate(this, true);
            while (true)
            {
                m_Task->Run(&delegate);
                if (WaitForRunOpportunity() == 0)
                {
                    return;
                }
            }
        }

        void V8JobState::CancelAndWait()
        {
            std::unique_lock<std::mutex> lock(m_Lock);
            m_Canceled.store(true, std::memory_order::relaxed);
            while (m_ActiveTasks > 0)
            {
                m_WorkerReleased.wait(lock);
            }
        }

        void V8JobState::CancelAndDetach()
        {
            m_Canceled.store(true, std::memory_order::relaxed);
        }

        bool V8JobState::IsActive()
        {
            std::unique_lock<std::mutex> lock(m_Lock);
            return m_Task->GetMaxConcurrency(m_ActiveTasks) != 0 || m_ActiveTasks != 0;
        }

        bool V8JobState::CanRunFirstTask()
        {
            std::unique_lock<std::mutex> lock(m_Lock);
            m_PendingTasks--;
            if (m_Canceled.load(std::memory_order::relaxed))
            {
                return false;
            }
            if (m_ActiveTasks >= GetMaxConcurrency(m_ActiveTasks))
            {
                return false;
            }
            m_ActiveTasks++;
            return true;
        }

        bool V8JobState::DidRunFirstTask()
        {
            v8::TaskPriority prioirty;
            {
                std::unique_lock<std::mutex> lock(m_Lock);
                prioirty = m_Priority;
                size_t maxConcurrency = GetMaxConcurrency(m_ActiveTasks - 1);
                if (m_Canceled.load(std::memory_order::relaxed) || m_ActiveTasks > maxConcurrency)
                {
                    m_ActiveTasks--;
                    m_WorkerReleased.notify_one();
                    return false;
                }
            }
            PostonWorkerThread(ComputeTaskToPost(GetMaxConcurrency(m_ActiveTasks - 1)), prioirty);
            return true;
        }

        void V8JobState::UpdatePriority(v8::TaskPriority inPriority)
        {
            std::unique_lock<std::mutex> lock(m_Lock);
            m_Priority = inPriority;
        }

        size_t V8JobState::ComputeTaskToPost(size_t inMaxConcurrency)
        {
            std::unique_lock<std::mutex> lock(m_Lock);
            if (inMaxConcurrency > m_ActiveTasks + m_PendingTasks)
            {
                inMaxConcurrency -= m_ActiveTasks + m_PendingTasks;
                m_PendingTasks += inMaxConcurrency;
                return inMaxConcurrency;
            }
            return 0;
        }

        void V8JobState::PostonWorkerThread(size_t inNumToPost, v8::TaskPriority inPriority)
        {
            for (int i = 0; i < inNumToPost; i++)
            {
                std::unique_ptr<V8JobTaskWorker> worker = std::make_unique<V8JobTaskWorker>(shared_from_this(), m_Task.get());
                switch (inPriority)
                {
                case v8::TaskPriority::kBestEffort:
                    m_Platform->CallLowPriorityTaskOnWorkerThread(std::move(worker));
                    break;
                case v8::TaskPriority::kUserVisible:
                    m_Platform->CallOnWorkerThread(std::move(worker));
                    break;
                case v8::TaskPriority::kUserBlocking:
                    m_Platform->CallBlockingTaskOnWorkerThread(std::move(worker));
                    break;
                }
            }
        }

        uint8_t V8JobState::FindFirstFreeTaskId(V8JobTaskIdType inIds)
        {
            for (uint8_t idx = 0; idx < V8JobTaskBits; idx++)
            {
                if ((1 << idx) & inIds)
                {
                    continue;
                }
                return idx;
            }
            return kInvalidJobId;
        }

        V8JobHandle::V8JobHandle(std::shared_ptr<V8JobState> inState) : m_State(std::move(inState))
        {
        }

        V8JobHandle::~V8JobHandle()
        {
            // DCHECK_EQ(nullptr, m_State);
        }

        void V8JobHandle::NotifyConcurrencyIncrease()
        {
            m_State->NotifyConcurrencyIncrease();
        }

        void V8JobHandle::Join()
        {
            m_State->Join();
            m_State = nullptr;
        }

        void V8JobHandle::Cancel()
        {
            m_State->CancelAndWait();
            m_State = nullptr;
        }

        void V8JobHandle::CancelAndDetach()
        {
            m_State->CancelAndDetach();
            m_State = nullptr;
        }

        bool V8JobHandle::IsActive()
        {
            return m_State->IsActive();
        }

        bool V8JobHandle::IsValid()
        {
            return m_State != nullptr;
        }

        bool V8JobHandle::UpdatePriorityEnabled() const
        {
            return true;
        }

        void V8JobHandle::UpdatePriority(v8::TaskPriority inPrioirty)
        {
            m_State->UpdatePriority(inPrioirty);
        }

        V8JobTaskWorker::V8JobTaskWorker(std::weak_ptr<V8JobState> inState, v8::JobTask *inTask) : m_State(inState), m_Task(inTask)
        {
        }

        V8JobTaskWorker::~V8JobTaskWorker()
        {
        }

        void V8JobTaskWorker::Run()
        {
            auto sharedState = m_State.lock();
            if (sharedState == nullptr)
            {
                return;
            }
            if (sharedState->CanRunFirstTask() == false)
            {
                return;
            }
            do
            {
                V8JobState::V8JobDelegate delegate(sharedState.get());
                m_Task->Run(&delegate);
            } while (sharedState->DidRunFirstTask());
        }

    }
}