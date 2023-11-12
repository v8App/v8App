//#include "TThreadSafeQueue.h"

namespace v8App {
    namespace Queues {
        template<class QueueType>
        TThreadSafeQueue<QueueType>::TThreadSafeQueue() : m_Terminated(false)
        {
        }

        template<class QueueType>
        TThreadSafeQueue<QueueType>::~TThreadSafeQueue()
        {
            std::lock_guard<std::mutex> lock(m_QueueLock);
            std::deque<QueueType> empty;
            std::swap(m_Queue, empty);
        }

        template<class QueueType>
        void TThreadSafeQueue<QueueType>::PushItem(QueueType inTask)
        {
            std::lock_guard<std::mutex> lock(m_QueueLock);
            if(m_Terminated)
            {
                return;
            }
            m_Queue.push_back(std::move(inTask));
        }

        template<class QueueType>
        std::optional<QueueType> TThreadSafeQueue<QueueType>::GetNextItem()
        {
            std::lock_guard<std::mutex> lock(m_QueueLock);
            if(m_Terminated || m_Queue.empty())
            {
                return {};
            }
            QueueType temp = std::move(m_Queue.front());
            m_Queue.pop_front();
            return temp;
        }

        template<class QueueType>
        void TThreadSafeQueue<QueueType>::Terminate()
        {
            std::lock_guard<std::mutex> lock(m_QueueLock);
            m_Terminated = true;
        }
    }
}