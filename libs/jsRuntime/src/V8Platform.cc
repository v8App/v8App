// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <future>
#include <functional>
#include <chrono>

#include "Time/Time.h"
#include "Logging/LogMacros.h"
#include "V8Platform.h"
#include "V8Jobs.h"
#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        std::shared_ptr<V8Platform> V8Platform::s_Platform;

        V8Platform::V8Platform()
        {
            m_TracingController = std::make_unique<v8::TracingController>();
            int cores = std::thread::hardware_concurrency() - 1;
            m_NumberOfWorkers = std::max(1, cores);
            for (int idx = 0; idx <= static_cast<int>(Threads::ThreadPriority::kMaxPriority); idx++)
            {
                m_WorkerRunners[idx] = std::make_shared<WorkerTaskRunner>(m_NumberOfWorkers, IntToPriority(idx));
            }
        }

        V8Platform::~V8Platform()
        {
            for (int idx = 0; idx <= static_cast<int>(Threads::ThreadPriority::kMaxPriority); idx++)
            {
                m_WorkerRunners[idx]->Terminate();
            }
        }

        // v8::platform interface
        v8::PageAllocator *V8Platform::GetPageAllocator()
        {
            return m_PageAllocator.get();
        }

        v8::ThreadIsolatedAllocator *V8Platform::GetThreadIsolatedAllocator()
        {
            return m_ThreadIsolatedAllocator.get();
        }

        v8::ZoneBackingAllocator *V8Platform::GetZoneBackingAllocator()
        {
            if (m_ZoneBlockingAllocator == nullptr)
            {
                return v8::Platform::GetZoneBackingAllocator();
            }
            return m_ZoneBlockingAllocator.get();
        }

        void V8Platform::OnCriticalMemoryPressure()
        {
            // for now we do nothing
        }

        int V8Platform::NumberOfWorkerThreads()
        {
            return m_NumberOfWorkers;
        }

        std::shared_ptr<v8::TaskRunner> V8Platform::GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority inPriority)
        {
            // make sure it's not null in debug
            //JSRuntimeSharedPtr runtime(JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate));
            //DCHECK_NE(runtime, nullptr);
            //return runtime->GetForegroundTaskRunner();
            return std::shared_ptr<ForegroundTaskRunner>();
        }

        bool V8Platform::IdleTasksEnabled(v8::Isolate *inIsolate)
        {
            // make sure it's not null in debug
            //JSRuntimeSharedPtr runtime(JSRuntime::GetJSRuntimeFromV8Isolate(inIsolate));
            //DCHECK_NE(runtime, nullptr);
            //return runtime->AreIdleTasksEnabled();
            return true;
        }

        std::unique_ptr<v8::ScopedBlockingCall> V8Platform::CreateBlockingScope(v8::BlockingType blocking_type)
        {
            // For now return nullptr
            return nullptr;
        }

        double V8Platform::MonotonicallyIncreasingTime()
        {
            return Time::MonotonicallyIncreasingTimeSeconds();
        }

        double V8Platform::CurrentClockTimeMillis()
        {
            return Time::MonotonicallyIncreasingTimeMilliSeconds();
        }

        V8Platform::StackTracePrinter V8Platform::GetStackTracePrinter()
        {
            // for now just return nullptr
            return nullptr;
        }

        v8::TracingController *V8Platform::GetTracingController()
        {
            DCHECK_NOT_NULL(m_TracingController);
            return m_TracingController.get();
        }

        void V8Platform::DumpWithoutCrashing()
        {
            // For now do nothing
        }

        v8::HighAllocationThroughputObserver *V8Platform::GetHighAllocationThroughputObserver()
        {
            // for now call the base
            if (m_HighAllocObserver == nullptr)
            {
                return v8::Platform::GetHighAllocationThroughputObserver();
            }
            return m_HighAllocObserver.get();
        }

        void V8Platform::SetTracingController(v8::TracingController *inController)
        {
            if (m_V8Inited != false && inController != nullptr)
            {
                m_TracingController.reset(inController);
            }
        }

        void V8Platform::SetPageAllocator(v8::PageAllocator *inAllocator)
        {
            if (m_V8Inited != false && inAllocator != nullptr)
            {
                m_PageAllocator.reset(inAllocator);
            }
        }

        void V8Platform::SetThreadIsolatatedAllocator(v8::ThreadIsolatedAllocator *inAllocator)
        {
            if (m_V8Inited != false && inAllocator != nullptr)
            {
                m_ThreadIsolatedAllocator.reset(inAllocator);
            }
        }

        void V8Platform::SetHighAllocatoionObserver(v8::HighAllocationThroughputObserver *inObserver)
        {
            if (m_V8Inited != false && inObserver != nullptr)
            {
                m_HighAllocObserver.reset(inObserver);
            }
        }

        void V8Platform::SetZoneBlockingAllocator(v8::ZoneBackingAllocator *inZoneAllocator)
        {
            if (m_V8Inited != false && inZoneAllocator != nullptr)
            {
                m_ZoneBlockingAllocator.reset(inZoneAllocator);
            }
        }

        void V8Platform::InitializeV8()
        {
            if (s_Platform != nullptr && s_Platform->m_V8Inited)
            {
                return;
            }

            v8::V8::InitializePlatform(Get().get());
            v8::V8::Initialize();
            s_Platform->m_V8Inited = true;
        }

        void V8Platform::ShutdownV8()
        {
            v8::V8::Dispose();
            v8::V8::DisposePlatform();
            s_Platform.reset();
        }

        std::shared_ptr<V8Platform> V8Platform::Get()
        {
            if (s_Platform == nullptr)
            {
                s_Platform = std::make_shared<V8Platform>();
            }
            return s_Platform;
        }

        std::unique_ptr<v8::JobHandle> V8Platform::CreateJobImpl(
            v8::TaskPriority priority, std::unique_ptr<v8::JobTask> job_task,
            const v8::SourceLocation &location)
        {
            size_t numWorkers = NumberOfWorkerThreads();
            if(priority == v8::TaskPriority::kBestEffort || numWorkers >2)
            {
                numWorkers = 2;
            }
            return std::make_unique<V8JobHandle>(std::make_shared<V8JobState>(this, std::move(job_task), priority, numWorkers));
        }

        void V8Platform::PostTaskOnWorkerThreadImpl(v8::TaskPriority priority,
                                        std::unique_ptr<v8::Task> task,
                                        const v8::SourceLocation &location)
        {
            int idx = PriorityToInt(priority);
            m_WorkerRunners[idx]->PostTask(std::move(task));

        }
        void V8Platform::PostDelayedTaskOnWorkerThreadImpl(
            v8::TaskPriority priority, std::unique_ptr<v8::Task> task,
            double delay_in_seconds, const v8::SourceLocation &location)
        {
            int idx = PriorityToInt(priority);
            m_WorkerRunners[idx]->PostDelayedTask(std::move(task), delay_in_seconds);
        }

    } // namespace JSRuntime
} // namespace v8App