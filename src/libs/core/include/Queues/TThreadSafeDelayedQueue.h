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

        /**
         * Implements a thread safe delayed queue.
         * The queue needs to be ticked periodically by calling MayHaveItems aso that
         * so that the delyed items can be checked and moved to the main queue once
         * their delay has expired.
         * Each of the operations Push, Get and MayHave all check the delayed items
         * to see if they can moved to the main queue for work.
        */
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

            /**
             * Push an item onto the queue with a delay and checks to see if any delayed items are 
             * ready and if moves them to the main queue
            */
            virtual void PushItemDelayed(double inDelaySeconds, QueueType inItem);
            /**
             * Gets the next item it could be that no item is returned as another thread may have 
             * already fetched the item. This also checks if any delayed item is ready and moves
             * it to the main queue
             */
            std::optional<QueueType> GetNextItem() override;
            /**
             * Checks to see if there are any items ready to be fetched. This also checks to see if any 
             * of the delayed items and moves them to the main queue.
             * This function should be periodiaclly ticked to process the delayed items
            */
            virtual bool MayHaveItems() override;

            /**
             * A callback to let someone know that items are ready to be fetched
            */
            void SetDelayedJobsReadyDelegate(DelayedJobsReadyDelegate inCallback)
            {
                m_DelayedJobsReady = inCallback;
            }

        protected:
            /**
             * Performs the actual work of checking and moving the delyaed items to the main queue
            */
            virtual void ProcessDelayedQueue();

            DelayedJobsReadyDelegate m_DelayedJobsReady;
            std::mutex m_DelayedLock;
            std::multimap<double, QueueType> m_DelayedQueue;
        };
    } // namespace Queues
} // namespace v8App

#include "TThreadSafeDelayedQueue.hpp"
#endif //__T_THREAD_SAFE_DELAYED_QUEUE_H_