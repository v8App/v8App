// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __NESTABLE__QUEUE_H_
#define __NESTABLE__QUEUE_H_

#include <queue>
#include <mutex>
#include <map>

#include "Queues/TThreadSafeDelayedQueue.h"
#include "Time/Time.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace internal
        {
            enum class Nestability
            {
                kNestable,
                kNonstable
            };

            struct QueueEntry
            {
                Nestability m_Nestable;
                V8TaskUniquePtr m_Task;
            };
        }

        class NestableQueue : protected Queues::TThreadSafeDelayedQueue<internal::QueueEntry>
        {
        public:
            NestableQueue();
            virtual ~NestableQueue();

            NestableQueue(const NestableQueue &) = delete;
            NestableQueue &operator=(const NestableQueue &) = delete;

            NestableQueue(NestableQueue &&inQueue)
            {
                this->m_Queue = std::move(inQueue.m_Queue);
            }

            void PushItem(V8TaskUniquePtr inItem);
            void PushItemDelayed(double inDelaySeconds, V8TaskUniquePtr inItem);

            void PushNonNestableItem(V8TaskUniquePtr inItem);
            void PushNonNestableItemDelayed(double inDelaySeconds, V8TaskUniquePtr inItem);

            std::optional<V8TaskUniquePtr> GetNextItem(int inNestingDepth);
            virtual bool MayHaveItems() override;

            virtual void Terminate() override;

        private:
            using TThreadSafeDelayedQueue::PushItem;
            using TThreadSafeDelayedQueue::PushItemDelayed;
            using TThreadSafeDelayedQueue::GetNextItem;
        };
    } // namespace Queues
} // namespace v8App

#endif //__NESTABLE__QUEUE_H_