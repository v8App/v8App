// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __T_THREAD_SAFE_DELAYED_QUEUE_H_
#define __T_THREAD_SAFE_DELAYED_QUEUE_H_

#include <queue>
#include <mutex>
#include <map>
#include <functional>

#include "TThreadSafeQueue.h"

namespace v8App
{
    namespace Queues
    {
        using DelayedJobsReadyDelegate = std::function<void()>;

        template <class QueueType>
        class TThreadSafeDelayedQueue : public TThreadSafeQueue<QueueType>
        {
        public:
            TThreadSafeDelayedQueue();
            virtual ~TThreadSafeDelayedQueue();

            TThreadSafeDelayedQueue(const TThreadSafeDelayedQueue &) = delete;
            TThreadSafeDelayedQueue &operator=(const TThreadSafeDelayedQueue &) = delete;

            TThreadSafeDelayedQueue(TThreadSafeDelayedQueue &&inQueue)
            {
                this->m_Queue = std::move(inQueue.m_Queue);
                this->m_DelayedQueue = std::move(inQueue.m_DelayedQueue);
            }

            virtual void PushItemDelayed(double inDelaySeconds, QueueType inItem);
            std::optional<QueueType> GetNextItem() override;
            virtual bool MayHaveItems() override;

            void SetDelayedJobsReadyDelegate(DelayedJobsReadyDelegate inCallback)
            {
                m_DelayedJobsReady = inCallback;
            }

        protected:
            virtual void ProcessDelayedQueue();

            DelayedJobsReadyDelegate m_DelayedJobsReady;
            std::mutex m_DelayedLock;
            std::multimap<double, QueueType> m_DelayedQueue;
        };
    } // namespace Queues
} // namespace v8App

#include "TThreadSafeDelayedQueue.hpp"
#endif //__T_THREAD_SAFE_DELAYED_QUEUE_H_