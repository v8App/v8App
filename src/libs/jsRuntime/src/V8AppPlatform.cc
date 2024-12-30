// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <future>
#include <functional>
#include <chrono>

#include "Time/Time.h"
#include "Logging/LogMacros.h"
#include "Logging/Log.h"
#include "V8AppPlatform.h"
#include "V8Jobs.h"
#include "v8/v8.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        std::shared_ptr<V8AppPlatform> V8AppPlatform::s_Platform;
        bool V8AppPlatform::s_PlatformDestoryed = false;
        bool V8AppPlatform::s_PlatformInited = false;

        V8AppPlatform::V8AppPlatform()
        {
            m_TracingController = std::make_unique<V8TracingController>();
            int cores = Threads::GetHardwareCores();
            m_NumberOfWorkers = std::max(1, cores);
            for (int idx = 0; idx <= static_cast<int>(Threads::ThreadPriority::kMaxPriority); idx++)
            {
                m_WorkerRunners[idx] = std::make_shared<WorkerTaskRunner>(m_NumberOfWorkers, IntToPriority(idx));
            }
        }

        V8AppPlatform::~V8AppPlatform()
        {
        }

        // v8::platform interface
        V8PageAllocator *V8AppPlatform::GetPageAllocator()
        {
            return m_PageAllocator.get();
        }

        V8ThreadIsolatedAllocator *V8AppPlatform::GetThreadIsolatedAllocator()
        {
            return m_ThreadIsolatedAllocator.get();
        }

        void V8AppPlatform::OnCriticalMemoryPressure()
        {
            // for now we do nothing
        }

        int V8AppPlatform::NumberOfWorkerThreads()
        {
            return m_NumberOfWorkers;
        }

        V8TaskRunnerSharedPtr V8AppPlatform::GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority inPriority)
        {
            // make sure it's not null in debug
            DCHECK_NE(m_IsolateHelper, nullptr);
            return m_IsolateHelper->GetForegroundTaskRunner(inIsolate, inPriority);
        }

        bool V8AppPlatform::IdleTasksEnabled(V8Isolate *inIsolate)
        {
            // make sure it's not null in debug
            DCHECK_NE(m_IsolateHelper, nullptr);
            return m_IsolateHelper->IdleTasksEnabled(inIsolate);
        }

        std::unique_ptr<v8::ScopedBlockingCall> V8AppPlatform::CreateBlockingScope(V8BlockingType blocking_type)
        {
            // For now return nullptr
            return nullptr;
        }

        double V8AppPlatform::MonotonicallyIncreasingTime()
        {
            return Time::MonotonicallyIncreasingTimeSeconds();
        }

        double V8AppPlatform::CurrentClockTimeMillis()
        {
            return Time::MonotonicallyIncreasingTimeMilliSeconds();
        }

        V8Platform::StackTracePrinter V8AppPlatform::GetStackTracePrinter()
        {
            // for now just return nullptr
            return nullptr;
        }

        V8TracingController *V8AppPlatform::GetTracingController()
        {
            DCHECK_NOT_NULL(m_TracingController);
            return m_TracingController.get();
        }

        void V8AppPlatform::DumpWithoutCrashing()
        {
            // For now do nothing
        }

        V8HighAllocationThroughputObserver *V8AppPlatform::GetHighAllocationThroughputObserver()
        {
            // for now call the base
            if (m_HighAllocObserver == nullptr)
            {
                return V8Platform::GetHighAllocationThroughputObserver();
            }
            return m_HighAllocObserver.get();
        }

        void V8AppPlatform::SetTracingController(V8TracingController *inController)
        {
            if (s_PlatformInited != false && inController != nullptr)
            {
                m_TracingController.reset(inController);
            }
        }

        void V8AppPlatform::SetPageAllocator(V8PageAllocator *inAllocator)
        {
            if (s_PlatformInited != false && inAllocator != nullptr)
            {
                m_PageAllocator.reset(inAllocator);
            }
        }

        void V8AppPlatform::SetThreadIsolatatedAllocator(V8ThreadIsolatedAllocator *inAllocator)
        {
            if (s_PlatformInited != false && inAllocator != nullptr)
            {
                m_ThreadIsolatedAllocator.reset(inAllocator);
            }
        }

        void V8AppPlatform::SetHighAllocatoionObserver(V8HighAllocationThroughputObserver *inObserver)
        {
            if (s_PlatformInited != false && inObserver != nullptr)
            {
                m_HighAllocObserver.reset(inObserver);
            }
        }

        void V8AppPlatform::SetIsolateHelper(PlatformRuntimeProviderUniquePtr inHelper)
        {
            m_IsolateHelper = std::move(inHelper);
        }

        void V8AppPlatform::InitializeV8(PlatformRuntimeProviderUniquePtr inHelper)
        {
            if (s_Platform != nullptr && s_PlatformInited)
            {
                return;
            }
            if (s_PlatformDestoryed)
            {
                Log::LogMessage message;
                message.emplace(Log::MsgKey::Msg, "Tried to initialize the V8 Platform after it's been destroyed");
                // this will abort the app
                Log::Log::Fatal(message);
                ABORT();
                return;
            }
            Get()->SetIsolateHelper(std::move(inHelper));
            v8::V8::InitializePlatform(Get().get());
            v8::V8::Initialize();
            cppgc::InitializeProcess(s_Platform->GetPageAllocator());
            s_PlatformInited = true;
        }

        void V8AppPlatform::ShutdownV8()
        {
            if (s_PlatformDestoryed || s_Platform == nullptr)
            {
                return;
            }
            for (int idx = 0; idx <= static_cast<int>(Threads::ThreadPriority::kMaxPriority); idx++)
            {
                s_Platform->m_WorkerRunners[idx]->Terminate();
            }
            if (s_PlatformInited)
            {
                cppgc::ShutdownProcess();
                v8::V8::Dispose();
                v8::V8::DisposePlatform();
            }
            s_PlatformDestoryed = true;
            s_PlatformInited = false;
            s_Platform.reset();
        }

        std::shared_ptr<V8AppPlatform> V8AppPlatform::Get()
        {
            if (s_Platform == nullptr)
            {
                s_Platform = std::make_shared<V8AppPlatform>();
            }
            return s_Platform;
        }

        bool V8AppPlatform::SetWorkersPaused(bool inPaused)
        {
            for(int idx = 0; idx < static_cast<int>(Threads::ThreadPriority::kMaxPriority) + 1; idx++)
            {
                m_WorkerRunners[idx]->SetPaused(inPaused);
            }
            return true;
        }

        V8JobHandleUniquePtr V8AppPlatform::CreateJobImpl(
            V8TaskPriority priority, std::unique_ptr<v8::JobTask> job_task,
            const V8SourceLocation &location)
        {
            size_t numWorkers = NumberOfWorkerThreads();
            if (priority == V8TaskPriority::kBestEffort || numWorkers > 2)
            {
                numWorkers = 2;
            }
            return std::make_unique<V8JobHandle>(std::make_shared<V8JobState>(this, std::move(job_task), priority, numWorkers));
        }

        void V8AppPlatform::PostTaskOnWorkerThreadImpl(V8TaskPriority priority,
                                                    V8TaskUniquePtr task,
                                                    const V8SourceLocation &location)
        {
            int idx = PriorityToInt(priority);
            m_WorkerRunners[idx]->PostTask(std::move(task));
        }
        void V8AppPlatform::PostDelayedTaskOnWorkerThreadImpl(
            V8TaskPriority priority, V8TaskUniquePtr task,
            double delay_in_seconds, const V8SourceLocation &location)
        {
            int idx = PriorityToInt(priority);
            m_WorkerRunners[idx]->PostDelayedTask(std::move(task), delay_in_seconds);
        }

    } // namespace JSRuntime
} // namespace v8App