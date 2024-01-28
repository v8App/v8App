// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8_PLATFORM_H_
#define _V8_PLATFORM_H_

#include <map>
#include "v8-platform.h"
#include "ForegroundTaskRunner.h"
#include "WorkerTaskRunner.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * A Helper class to remove a reference to the JSRuntime and isolate for testing.
        */
        class PlatformIsolateHelper
        {
            public:
            virtual ~PlatformIsolateHelper() = default;
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority priority) = 0;
            virtual bool IdleTasksEnabled(v8::Isolate *inIsolate) = 0;
        };

        using PlatformIsolateHelperUniquePtr = std::unique_ptr<PlatformIsolateHelper>;
        
         class V8Platform : public v8::Platform
        {
        public:
            V8Platform();
            ~V8Platform();

            // v8::platform interface
            virtual v8::PageAllocator *GetPageAllocator() override;
            virtual v8::ThreadIsolatedAllocator *GetThreadIsolatedAllocator() override;
            virtual v8::ZoneBackingAllocator *GetZoneBackingAllocator() override;
            virtual void OnCriticalMemoryPressure() override;
            virtual int NumberOfWorkerThreads() override;
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority priority) override;

            virtual bool IdleTasksEnabled(v8::Isolate *inIsolate) override;

            virtual std::unique_ptr<v8::ScopedBlockingCall> CreateBlockingScope(v8::BlockingType blocking_type) override;

            // retain virtual so that testing can override it.
            virtual double MonotonicallyIncreasingTime() override;
            virtual double CurrentClockTimeMillis() override;

            virtual StackTracePrinter GetStackTracePrinter() override;
            virtual v8::TracingController *GetTracingController() override;
            virtual void DumpWithoutCrashing() override;
            virtual v8::HighAllocationThroughputObserver *GetHighAllocationThroughputObserver() override;
            // end v8::platform interface

            void SetTracingController(v8::TracingController *inController);
            void SetPageAllocator(v8::PageAllocator *inAllocator);
            void SetThreadIsolatatedAllocator(v8::ThreadIsolatedAllocator *inAllocator);
            void SetHighAllocatoionObserver(v8::HighAllocationThroughputObserver *inObserver);
            void SetZoneBlockingAllocator(v8::ZoneBackingAllocator *inZoneAllocator);

            void SetIsolateHelper(PlatformIsolateHelperUniquePtr inHelper);

            static void InitializeV8(PlatformIsolateHelperUniquePtr inHelper);
            static void ShutdownV8();

            static std::shared_ptr<V8Platform> Get();

        protected:
            V8Platform(const V8Platform &) = delete;
            V8Platform &operator=(const V8Platform &) = delete;

            // v8::platform interface
            virtual std::unique_ptr<v8::JobHandle> CreateJobImpl(
                v8::TaskPriority priority, std::unique_ptr<v8::JobTask> job_task,
                const v8::SourceLocation &location) override;
            virtual void PostTaskOnWorkerThreadImpl(v8::TaskPriority priority,
                                                    std::unique_ptr<v8::Task> task,
                                                    const v8::SourceLocation &location) override;
            virtual void PostDelayedTaskOnWorkerThreadImpl(
                v8::TaskPriority priority, std::unique_ptr<v8::Task> task,
                double delay_in_seconds, const v8::SourceLocation &location) override;
            // end v8::platform interface

            inline Threads::ThreadPriority IntToPriority(int inInt)
            {
                if (inInt < 0 || inInt > static_cast<int>(Threads::ThreadPriority::kMaxPriority))
                {
                    return Threads::ThreadPriority::kBestEffort;
                }
                return static_cast<Threads::ThreadPriority>(inInt);
            }

            inline int PriorityToInt(v8::TaskPriority inPriority)
            {
                switch (inPriority)
                {
                case v8::TaskPriority::kUserVisible:
                    return static_cast<int>(Threads::ThreadPriority::kUserVisible);
                case v8::TaskPriority::kUserBlocking:
                    return static_cast<int>(Threads::ThreadPriority::kUserBlocking);
                default:
                    return static_cast<int>(Threads::ThreadPriority::kBestEffort);
                }
            }

            std::shared_ptr<WorkerTaskRunner> m_WorkerRunners[static_cast<int>(Threads::ThreadPriority::kMaxPriority) + 1] = {0};

            std::unique_ptr<v8::TracingController> m_TracingController;
            std::unique_ptr<v8::PageAllocator> m_PageAllocator;
            std::unique_ptr<v8::ThreadIsolatedAllocator> m_ThreadIsolatedAllocator;
            std::unique_ptr<v8::HighAllocationThroughputObserver> m_HighAllocObserver;
            std::unique_ptr<v8::ZoneBackingAllocator> m_ZoneBlockingAllocator;

            int m_NumberOfWorkers;

            static bool s_PlatformDestoryed;
            static bool s_PlatformInited;

            PlatformIsolateHelperUniquePtr m_IsolateHelper;

            static std::shared_ptr<V8Platform> s_Platform;
        };
    } // namespace JSRuntime
} // namespace v8App
#endif
