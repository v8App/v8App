// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "NestableQueue.h"

namespace v8App
{
    namespace JSRuntime
    {
        using DelayedQueue = Queues::TThreadSafeDelayedQueue<internal::QueueEntry>;
        using QueueEntry = internal::QueueEntry;
        using Nestability = internal::Nestability;

        NestableQueue::NestableQueue() : DelayedQueue()
        {
        }
        NestableQueue::~NestableQueue()
        {
        }

        void NestableQueue::PushItem(V8TaskUniquePtr inItem)
        {
            internal::QueueEntry temp;
            temp.m_Nestable = Nestability::kNestable;
            temp.m_Task = std::move(inItem);
            PushItem(std::move(temp));
        }
        void NestableQueue::PushItemDelayed(double inDelaySeconds, V8TaskUniquePtr inItem)
        {
            internal::QueueEntry temp;
            temp.m_Nestable = Nestability::kNestable;
            temp.m_Task = std::move(inItem);
            PushItemDelayed(inDelaySeconds, std::move(temp));
        }

        void NestableQueue::PushNonNestableItem(V8TaskUniquePtr inItem)
        {
            internal::QueueEntry temp;
            temp.m_Nestable = Nestability::kNonstable;
            temp.m_Task = std::move(inItem);
            PushItem(std::move(temp));
        }
        void NestableQueue::PushNonNestableItemDelayed(double inDelaySeconds, V8TaskUniquePtr inItem)
        {
            internal::QueueEntry temp;
            temp.m_Nestable = Nestability::kNonstable;
            temp.m_Task = std::move(inItem);
            PushItemDelayed(inDelaySeconds, std::move(temp));
        }

        std::optional<V8TaskUniquePtr> NestableQueue::GetNextItem(int inNestingDepth)
        {
            if (m_Terminated)
            {
                return {};
            }

            ProcessDelayedQueue();
            std::lock_guard<std::mutex> lock(m_DelayedLock);
            auto it = m_Queue.begin();
            while (it != m_Queue.end())
            {
                if (it->m_Nestable == Nestability::kNonstable && inNestingDepth != 0)
                {
                    it++;
                    continue;
                }
                V8TaskUniquePtr temp = std::move(it->m_Task);
                //it->m_Task.release();
                m_Queue.erase(it);
                return std::move(temp);
            }
            return {};
        }

        bool NestableQueue::MayHaveItems()
        {
            return DelayedQueue::MayHaveItems();
        }

        void NestableQueue::Terminate()
        {
            DelayedQueue::Terminate();
        }
    }
}