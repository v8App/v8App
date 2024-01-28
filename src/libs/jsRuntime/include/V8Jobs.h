// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8_JOBS_H_
#define _V8_JOBS_H_

#include <mutex>
#include <atomic>
#include <memory>

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
// For 64 bit platforms use a 64 bit type allowing more task ids.
#ifdef PLATFORM_64
        using V8JobTaskIdAtomicType = std::atomic_uint64_t;
        using V8JobTaskIdType = utint64_t;
#else
        using V8JobTaskIdAtomicType = std::atomic_uint32_t;
        using V8JobTaskIdType = uint32_t;
#endif
        const size_t V8JobTaskBits = sizeof(V8JobTaskIdType) * 8;

        class V8JobState : public std::enable_shared_from_this<V8JobState>
        {
        public:
            class V8JobDelegate : public ::v8::JobDelegate
            {
            public:
                explicit V8JobDelegate(V8JobState *inState, bool isJoiningThread = false);
                ~V8JobDelegate();

                // v8::JobDelegate Implmentation
                virtual bool ShouldYield() override;
                virtual void NotifyConcurrencyIncrease() override;
                virtual uint8_t GetTaskId() override;
                virtual bool IsJoiningThread() const override;
                // end v8::JobDelegate Implmentation
            protected:
                uint8_t m_TaskId = V8JobState::kInvalidJobId;
                V8JobState *m_JobState;
                bool m_JoingThread;
                //for some resons on a mac this wanted to default to true.
                bool m_Yielded = {false};
            };

            V8JobState(v8::Platform *inPlatform, V8JobTaskUniquePtr inTask, v8::TaskPriority inPriority, size_t inNumWorkers);
            virtual ~V8JobState();

            void NotifyConcurrencyIncrease();
            uint8_t AcquireTaskId();
            void ReleaseTaskID(uint8_t inTaskId);

            void Join();
            void CancelAndWait();
            void CancelAndDetach();
            bool IsActive();

            bool CanRunFirstTask();
            bool DidRunFirstTask();

            void UpdatePriority(v8::TaskPriority inPriority);

        protected:
            size_t ComputeTaskToPost(size_t inMaxConcurrency);
            void PostonWorkerThread(size_t inNumToPost, v8::TaskPriority inPriority);

            inline size_t GetMaxConcurrency(size_t inWorkerCount)
            {
                return std::min(m_Task->GetMaxConcurrency(inWorkerCount), m_NumWorkersAvailable);
            }

            uint8_t FindFirstFreeTaskId(V8JobTaskIdType inIds);

            static constexpr V8JobTaskIdType kInvalidJobId = std::numeric_limits<uint8_t>::max();

            v8::Platform *m_Platform;
            V8JobTaskUniquePtr m_Task;

            std::mutex m_Lock;
            V8JobTaskIdAtomicType m_AssignedTaskIds;
            std::atomic_bool m_Canceled{false};
            v8::TaskPriority m_Priority;
            size_t m_ActiveTasks = 0;
            size_t m_PendingTasks = 0;
            size_t m_NumWorkersAvailable;
            std::condition_variable m_WorkerReleased;
        };

        class V8JobHandle : public v8::JobHandle
        {
        public:
            V8JobHandle(std::shared_ptr<V8JobState> inState);
            virtual ~V8JobHandle() override;

            V8JobHandle(const V8JobHandle &) = delete;
            V8JobHandle &operator=(const V8JobHandle &) = delete;

            // v8::JobHandle Implementation
            virtual void NotifyConcurrencyIncrease() override;
            virtual void Join() override;
            virtual void Cancel() override;
            virtual void CancelAndDetach() override;
            virtual bool IsActive() override;
            virtual bool IsValid() override;
            virtual bool UpdatePriorityEnabled() const override;
            virtual void UpdatePriority(v8::TaskPriority new_priority) override;
            // v8::JobHandle Implementation

        protected:
            std::shared_ptr<V8JobState> m_State;
        };

        class V8JobTaskWorker : public v8::Task
        {
        public:
            V8JobTaskWorker(std::weak_ptr<V8JobState> inState, v8::JobTask *inTask);
            ~V8JobTaskWorker();

            V8JobTaskWorker(const V8JobTaskWorker &) = delete;
            V8JobTaskWorker &operator=(const V8JobTaskWorker &) = delete;

            void Run() override;

        protected:
            std::weak_ptr<V8JobState> m_State;
            v8::JobTask *m_Task;
        };
    }
}
#endif //_V8_JOBS_H_