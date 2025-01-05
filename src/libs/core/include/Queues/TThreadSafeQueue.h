// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
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
        /**
         * Implements a thread safe queue
        */
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

            /**
             * Adds an item to the queue
             */
            virtual void PushItem(QueueType inItem);
            /**
             * Gets the next item it could be that no item is returned as another thread may have 
             * already fetched the item
             */
            virtual std::optional<QueueType> GetNextItem();

            /**
             * Check to see if there may be an item in the queue to get
            */
            virtual bool MayHaveItems() { return m_Queue.size() != 0;}

            /**
             * Set the queue to a teminating state preventing any additional items to be added
            */
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