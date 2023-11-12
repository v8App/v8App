
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

// #include "TThreadSafeDelayedQueue.h"
#include "Logging/LogMacros.h"
#include <iostream>
#include "Time/Time.h"

namespace v8App
{
    namespace Queues
    {
        template <class QueueType>
        TThreadSafeDelayedQueue<QueueType>::TThreadSafeDelayedQueue()
        {
        }

        template <class QueueType>
        TThreadSafeDelayedQueue<QueueType>::~TThreadSafeDelayedQueue()
        {
            std::lock_guard<std::mutex> lock(this->m_QueueLock);
            std::deque<QueueType> empty;
            std::swap(this->m_Queue, empty);
            std::multimap<double, QueueType> emptyDelated;
            std::swap(this->m_DelayedQueue, emptyDelated);
            this->m_Terminated = true;
        }

        template <class QueueType>
        void TThreadSafeDelayedQueue<QueueType>::PushItemDelayed(double inDelaySeconds, QueueType inItem)
        {
            DCHECK_GE(inDelaySeconds, 0.0);
            std::lock_guard lock(this->m_DelayedLock);
            if (this->m_Terminated)
            {
                return;
            }

            double deadline = Time::MonotonicallyIncreasingTimeSeconds() + inDelaySeconds;
            this->m_DelayedQueue.emplace(deadline, std::move(inItem));
        }

        template <class QueueType>
        std::optional<QueueType> TThreadSafeDelayedQueue<QueueType>::GetNextItem()
        {
            ProcessDelayedQueue();
            return TThreadSafeQueue<QueueType>::GetNextItem();
        }

        template <class QueueType>
        bool TThreadSafeDelayedQueue<QueueType>::MayHaveItems()
        {
            ProcessDelayedQueue();
            return TThreadSafeQueue<QueueType>::MayHaveItems();
        }

        template <class QueueType>
        void TThreadSafeDelayedQueue<QueueType>::ProcessDelayedQueue()
        {
            std::lock_guard<std::mutex> lock(this->m_DelayedLock);
            if (this->m_Terminated)
            {
                return;
            }
            // Move any items that have hit their dealyed time and push them onto the main queu
            if (this->m_DelayedQueue.empty() == false)
            {
                double now = Time::MonotonicallyIncreasingTimeSeconds();
                QueueType item;
                auto it = this->m_DelayedQueue.begin();
                bool jobsReady = false;
                while (it != this->m_DelayedQueue.end() && it->first <= now)
                {
                    jobsReady = true;
                    item = std::move(it->second);
                    this->m_DelayedQueue.erase(it);
                    TThreadSafeQueue<QueueType>::PushItem(std::move(item));
                    // since we modified the map we need the iterator
                    it = this->m_DelayedQueue.begin();
                }
                if(jobsReady && m_DelayedJobsReady)
                {
                    m_DelayedJobsReady();
                }
            }
        }
    } // namespace Queues
} // namespace v8App
