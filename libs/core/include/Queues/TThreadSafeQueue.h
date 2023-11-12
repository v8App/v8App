// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __T_THREAD_SAFE_QUEUE_H_
#define __T_THREAD_SAFE_QUEUE_H_

#include <queue>
#include <memory>
#include <optional>
#include <mutex>

namespace v8App
{
    namespace Queues
    {
        template <class QueueType>
        class TThreadSafeQueue
        {

        public:
            TThreadSafeQueue();
            virtual ~TThreadSafeQueue();

            TThreadSafeQueue(const TThreadSafeQueue &) = delete;
            TThreadSafeQueue &operator=(const TThreadSafeQueue &) = delete;

            TThreadSafeQueue(TThreadSafeQueue &&inQueue)
            {
                m_Queue = std::move(inQueue.m_Queue);
            }

            virtual void PushItem(QueueType inItem);
            virtual std::optional<QueueType> GetNextItem();

            virtual bool MayHaveItems() { return m_Queue.size() != 0;}

            virtual void Terminate();

        protected:
            bool m_Terminated = false;
            std::mutex m_QueueLock;
            std::deque<QueueType> m_Queue;
        };
    } // Queues
} // v8App

#include "TThreadSafeQueue.hpp"
#endif //__T_THREAD_SAFE_QUEUE_H_