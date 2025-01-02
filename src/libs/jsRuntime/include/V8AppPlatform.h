// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8APP_PLATFORM_H_
#define _V8APP_PLATFORM_H_

#include <map>

#include "v8/v8-platform.h"
#include "v8/cppgc/platform.h"

#include "ForegroundTaskRunner.h"
#include "WorkerTaskRunner.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that implments v8 platform for v8App
         */
        class V8AppPlatform : public v8::Platform
        {
        public:
            V8AppPlatform();
            ~V8AppPlatform();

            // v8::platform interface
            virtual V8PageAllocator *GetPageAllocator() override;
            virtual V8ThreadIsolatedAllocator *GetThreadIsolatedAllocator() override;
            virtual void OnCriticalMemoryPressure() override;
            virtual int NumberOfWorkerThreads() override;
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) override;

            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) override;

            virtual V8ScopedBlockingCallUniquePtr CreateBlockingScope(V8BlockingType blocking_type) override;

            // retain virtual so that testing can override it.
            virtual double MonotonicallyIncreasingTime() override;
            virtual double CurrentClockTimeMillis() override;

            virtual StackTracePrinter GetStackTracePrinter() override;
            virtual V8TracingController *GetTracingController() override;
            virtual void DumpWithoutCrashing() override;
            virtual V8HighAllocationThroughputObserver *GetHighAllocationThroughputObserver() override;
            // end v8::platform interface

            void SetTracingController(V8TracingController *inController);
            void SetPageAllocator(V8PageAllocator *inAllocator);
            void SetThreadIsolatatedAllocator(V8ThreadIsolatedAllocator *inAllocator);
            void SetHighAllocatoionObserver(V8HighAllocationThroughputObserver *inObserver);

            void SetIsolateHelper(PlatformRuntimeProviderUniquePtr inHelper);

            static void InitializeV8(PlatformRuntimeProviderUniquePtr inHelper);
            static void ShutdownV8();

            static std::shared_ptr<V8AppPlatform> Get();

            bool SetWorkersPaused(bool inPaused);

        protected:
            V8AppPlatform(const V8AppPlatform &) = delete;
            V8AppPlatform &operator=(const V8AppPlatform &) = delete;

            // v8::platform interface
            virtual V8JobHandleUniquePtr CreateJobImpl(
                V8TaskPriority priority, V8JobTaskUniquePtr job_task,
                const V8SourceLocation &location) override;
            virtual void PostTaskOnWorkerThreadImpl(V8TaskPriority priority,
                                                    V8TaskUniquePtr task,
                                                    const V8SourceLocation &location) override;
            virtual void PostDelayedTaskOnWorkerThreadImpl(
                V8TaskPriority priority, V8TaskUniquePtr task,
                double delay_in_seconds, const V8SourceLocation &location) override;
            // end v8::platform interface

            inline Threads::ThreadPriority IntToPriority(int inInt)
            {
                if (inInt < 0 || inInt > static_cast<int>(Threads::ThreadPriority::kMaxPriority))
                {
                    return Threads::ThreadPriority::kBestEffort;
                }
                return static_cast<Threads::ThreadPriority>(inInt);
            }

            inline int PriorityToInt(V8TaskPriority inPriority)
            {
                switch (inPriority)
                {
                case V8TaskPriority::kUserVisible:
                    return static_cast<int>(Threads::ThreadPriority::kUserVisible);
                case V8TaskPriority::kUserBlocking:
                    return static_cast<int>(Threads::ThreadPriority::kUserBlocking);
                default:
                    return static_cast<int>(Threads::ThreadPriority::kBestEffort);
                }
            }

            std::shared_ptr<WorkerTaskRunner> m_WorkerRunners[static_cast<int>(Threads::ThreadPriority::kMaxPriority) + 1] = {0};

            V8TracingControllerUniquePtr m_TracingController;
            V8PageAllocatorUniquePtr m_PageAllocator;
            V8ThreadIsolatedAllocatorUniquePtr m_ThreadIsolatedAllocator;
            V8HighAllocationThroughputObserverUniquePtr m_HighAllocObserver;

            int m_NumberOfWorkers;

            static bool s_PlatformDestroyed;
            static bool s_PlatformInited;

            PlatformRuntimeProviderUniquePtr m_IsolateHelper;

            static std::shared_ptr<V8AppPlatform> s_Platform;
        };
    } // namespace JSRuntime
} // namespace v8App
#endif //_V8APP_PLATFORM_H_
