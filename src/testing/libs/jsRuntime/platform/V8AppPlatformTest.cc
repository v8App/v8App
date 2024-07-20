
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8AppPlatform.h"
#include "IJSPlatformRuntimeProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestForegroundTaskRunner : public v8::TaskRunner
        {
        public:
            virtual void PostTask(V8TaskUniquePtr task) override {}

            virtual void PostDelayedTask(V8TaskUniquePtr task,
                                         double delay_in_seconds) override {}
            virtual void PostIdleTask(std::unique_ptr<v8::IdleTask> task) override {}
            virtual bool IdleTasksEnabled() override {return true;}
        };

        class TestIsolateHelper : public IJSPlatformRuntimeProvider
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) override
            {
                return std::make_shared<TestForegroundTaskRunner>();
            };
            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) override { return true; }
        };

        class TestPageAllocator : public v8::PageAllocator
        {
        public:
            TestPageAllocator() = default;
            size_t AllocatePageSize() override { return 0; }
            size_t CommitPageSize() override { return 0; }
            void SetRandomMmapSeed(int64_t seed) override{};
            void *GetRandomMmapAddr() override { return nullptr; }
            void *AllocatePages(void *address, size_t length, size_t alignment,
                                Permission permissions) override { return nullptr; }
            bool DecommitPages(void *address, size_t size) override { return false; }
            bool FreePages(void *address, size_t length) override { return false; }
            bool ReleasePages(void *address, size_t length,
                              size_t new_length) override { return false; }
            bool SetPermissions(void *address, size_t length,
                                Permission permissions) override { return false; }
        };

        class TestThreadIsolatedAllocator : public v8::ThreadIsolatedAllocator
        {
        public:
            TestThreadIsolatedAllocator() = default;
            void *Allocate(size_t size) override { return nullptr; }
            void Free(void *object) override {}
            enum Type Type() const override { return V8ThreadIsolatedAllocator::Type::kPkey; }
        };

        int testPlatformInt1 = 0;
        int testPlatformInt2 = 0;

        class TestPlatformTask : public v8::Task
        {
        public:
            TestPlatformTask(int *inPtr, int inValue) : intPtr(inPtr), value(inValue) {}
            void Run() { *intPtr = value; }
            int *intPtr;
            int value;
        };

        class TestV8AppPlatform : public V8AppPlatform
        {
        public:
            TestV8AppPlatform() {}

            Threads::ThreadPriority TestIntToPriority(int inInt) { return IntToPriority(inInt); }
            int TestPriorityToInt(V8TaskPriority inPriority) { return PriorityToInt(inPriority); }
            bool IsInited() { return s_PlatformInited; }
            void SetInited(bool inValue) { s_PlatformInited = inValue; }
        };

        class TestPlatformJobTask : public v8::JobTask
        {
        public:
            virtual void Run(v8::JobDelegate *delegate) override
            {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                m_Concurrency--;
            }
            virtual size_t GetMaxConcurrency(size_t worker_count) const override
            {
                return m_Concurrency;
            }
            size_t m_Concurrency = 2;
        };

        TEST(V8AppPlatformTest, ConstructorDestructor)
        {
            int cores = Threads::GetHardwareCores();
            TestV8AppPlatform platform;
            EXPECT_EQ(platform.NumberOfWorkerThreads(), cores);
            EXPECT_FALSE(platform.IsInited());
            EXPECT_EQ(platform.CreateBlockingScope(v8::BlockingType::kMayBlock), nullptr);
            EXPECT_EQ(platform.GetStackTracePrinter(), nullptr);
        }

        TEST(V8AppPlatformTest, GetSetTracingController)
        {
            TestV8AppPlatform platform;
            V8TracingController *constrcuted = platform.GetTracingController();
            EXPECT_NE(constrcuted, nullptr);

            platform.SetTracingController(nullptr);
            EXPECT_EQ(platform.GetTracingController(), constrcuted);

            platform.SetInited(true);
            platform.SetTracingController(nullptr);
            EXPECT_EQ(platform.GetTracingController(), constrcuted);

            V8TracingControllerUniquePtr controller = std::make_unique<V8TracingController>();
            platform.SetInited(false);
            platform.SetTracingController(controller.get());
            EXPECT_EQ(platform.GetTracingController(), constrcuted);

            platform.SetInited(true);
            platform.SetTracingController(controller.get());
            EXPECT_EQ(platform.GetTracingController(), controller.get());
            controller.release();
        }

        TEST(V8AppPlatformTest, GetSetPageAllocator)
        {
            TestV8AppPlatform platform;
            EXPECT_EQ(platform.GetPageAllocator(), nullptr);

            platform.SetInited(false);
            std::unique_ptr<TestPageAllocator> allocator = std::make_unique<TestPageAllocator>();
            platform.SetPageAllocator(allocator.get());
            EXPECT_EQ(platform.GetPageAllocator(), nullptr);

            platform.SetInited(true);
            platform.SetPageAllocator(allocator.get());
            EXPECT_EQ(platform.GetPageAllocator(), allocator.get());

            platform.SetInited(false);
            platform.SetPageAllocator(nullptr);
            EXPECT_EQ(platform.GetPageAllocator(), allocator.get());

            allocator.release();
        }

        TEST(V8AppPlatformTest, GetSetThreadIsolatedAllocator)
        {
            TestV8AppPlatform platform;
            EXPECT_EQ(platform.GetPageAllocator(), nullptr);

            std::unique_ptr<TestThreadIsolatedAllocator> allocator = std::make_unique<TestThreadIsolatedAllocator>();
            platform.SetThreadIsolatatedAllocator(allocator.get());
            EXPECT_EQ(platform.GetThreadIsolatedAllocator(), nullptr);

            platform.SetInited(true);
            platform.SetThreadIsolatatedAllocator(allocator.get());
            EXPECT_EQ(platform.GetThreadIsolatedAllocator(), allocator.get());

            platform.SetInited(false);
            platform.SetThreadIsolatatedAllocator(nullptr);
            EXPECT_EQ(platform.GetThreadIsolatedAllocator(), allocator.get());

            allocator.release();
        }

        TEST(V8AppPlatformTest, GetSetZoneBackingAllocator)
        {
            TestV8AppPlatform platform;
            V8ZoneBackingAllocator *constrcuted = platform.GetZoneBackingAllocator();
            EXPECT_NE(constrcuted, nullptr);

            V8ZoneBackingAllocatorUniquePtr allocator = std::make_unique<V8ZoneBackingAllocator>();
            platform.SetZoneBlockingAllocator(allocator.get());
            EXPECT_EQ(platform.GetZoneBackingAllocator(), constrcuted);

            platform.SetInited(true);
            platform.SetZoneBlockingAllocator(allocator.get());
            EXPECT_EQ(platform.GetZoneBackingAllocator(), allocator.get());

            platform.SetZoneBlockingAllocator(nullptr);
            EXPECT_EQ(platform.GetZoneBackingAllocator(), allocator.get());

            allocator.release();
        }

        TEST(V8AppPlatformTest, MonotonicallyIncreasingTime)
        {
            TestV8AppPlatform platform;
            // Make sure we're using the normal time function
            TestTime::TestTimeSeconds::Clear();
            double current = Time::MonotonicallyIncreasingTimeSeconds();
            EXPECT_DOUBLE_EQ(platform.MonotonicallyIncreasingTime(), current);
        }

        TEST(V8AppPlatformTest, CurrentClockTimeMilliseconds)
        {
            TestV8AppPlatform platform;
            // Make sure we're using the normal time function
            TestTime::TestTimeMilliSeconds::Clear();
            double current = Time::MonotonicallyIncreasingTimeMilliSeconds();
            EXPECT_DOUBLE_EQ(platform.CurrentClockTimeMilliseconds(), current);
        }

        TEST(V8AppPlatformTest, GetSetHighAllocationThroughputObserver)
        {
            TestV8AppPlatform platform;
            V8HighAllocationThroughputObserver *constrcuted = platform.GetHighAllocationThroughputObserver();
            EXPECT_NE(constrcuted, nullptr);

            platform.SetInited(false);
            V8HighAllocationThroughputObserverUniquePtr observer = std::make_unique<V8HighAllocationThroughputObserver>();
            platform.SetHighAllocatoionObserver(observer.get());
            EXPECT_EQ(platform.GetHighAllocationThroughputObserver(), constrcuted);

            platform.SetInited(true);
            platform.SetHighAllocatoionObserver(observer.get());
            EXPECT_EQ(platform.GetHighAllocationThroughputObserver(), observer.get());

            platform.SetHighAllocatoionObserver(nullptr);
            EXPECT_EQ(platform.GetHighAllocationThroughputObserver(), observer.get());

            observer.release();
        }

        TEST(V8AppPlatformTest, GetForegroundTaskRunner)
        {
            std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();
            PlatformRuntimeProviderUniquePtr helper = std::make_unique<TestIsolateHelper>();
            platform->SetIsolateHelper(std::move(helper));
            EXPECT_NE(nullptr, platform->GetForegroundTaskRunner(nullptr, V8TaskPriority::kBestEffort).get());
        }

        TEST(V8AppPlatformTest, IdleTasksEnabled)
        {
            std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();
            PlatformRuntimeProviderUniquePtr helper = std::make_unique<TestIsolateHelper>();
            platform->SetIsolateHelper(std::move(helper));
            EXPECT_TRUE(platform->IdleTasksEnabled(nullptr));
        }

        TEST(V8AppPlatformTest, Get)
        {
            std::shared_ptr<V8AppPlatform> platform = V8AppPlatform::Get();
            EXPECT_NE(platform, nullptr);
            EXPECT_EQ(platform, V8AppPlatform::Get());
        }

        TEST(V8AppPlatformTest, CreateJobImpl)
        {
            TestV8AppPlatform platform;
            platform.SetInited(false);

            V8JobHandleUniquePtr handle;
            std::unique_ptr<TestPlatformJobTask> task = std::make_unique<TestPlatformJobTask>();

            auto start = std::chrono::high_resolution_clock::now();
            handle = platform.PostJob(V8TaskPriority::kBestEffort, std::move(task));
            handle->Join();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(elapsed, 2);

            start = std::chrono::high_resolution_clock::now();
            task = std::make_unique<TestPlatformJobTask>();
            handle = platform.CreateJob(V8TaskPriority::kBestEffort, std::move(task));
            end = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_LE(elapsed, 1);

            std::this_thread::sleep_for(std::chrono::seconds(2));
            handle->NotifyConcurrencyIncrease();
            handle->Join();
            end = std::chrono::high_resolution_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(elapsed, 4);
        }

        TEST(V8AppPlatformTest, PostTaskOnWorkerThreadImpl)
        {
            TestV8AppPlatform platform;
            platform.SetInited(false);

            EXPECT_EQ(testPlatformInt1, 0);
            platform.CallOnWorkerThread(std::make_unique<TestPlatformTask>(&testPlatformInt1, 2));
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(testPlatformInt1, 2);

            platform.CallBlockingTaskOnWorkerThread(std::make_unique<TestPlatformTask>(&testPlatformInt1, 4));
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(testPlatformInt1, 4);

            platform.CallLowPriorityTaskOnWorkerThread(std::make_unique<TestPlatformTask>(&testPlatformInt1, 6));
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(testPlatformInt1, 6);
        }

        TEST(V8AppPlatformTest, PostDelayedTaskOnWorkerThreadImpl)
        {
            TestV8AppPlatform platform;
            platform.SetInited(false);

            EXPECT_EQ(testPlatformInt2, 0);
            platform.CallDelayedOnWorkerThread(std::make_unique<TestPlatformTask>(&testPlatformInt2, 2), 10);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(testPlatformInt2, 0);
            std::this_thread::sleep_for(std::chrono::seconds(10));
            EXPECT_EQ(testPlatformInt2, 2);
        }

        TEST(V8AppPlatformTest, IntToPriority)
        {
            TestV8AppPlatform platform;
            platform.SetInited(false);

            int max = static_cast<int>(Threads::ThreadPriority::kMaxPriority) + 1;

            EXPECT_EQ(Threads::ThreadPriority::kBestEffort, platform.TestIntToPriority(-1));
            EXPECT_EQ(Threads::ThreadPriority::kDefault, platform.TestIntToPriority(0));
            EXPECT_EQ(Threads::ThreadPriority::kBestEffort, platform.TestIntToPriority(1));
            EXPECT_EQ(Threads::ThreadPriority::kUserVisible, platform.TestIntToPriority(2));
            EXPECT_EQ(Threads::ThreadPriority::kUserBlocking, platform.TestIntToPriority(3));
            EXPECT_EQ(Threads::ThreadPriority::kBestEffort, platform.TestIntToPriority(max));
        }

        TEST(V8AppPlatformTest, PriorityToInt)
        {
            TestV8AppPlatform platform;
            platform.SetInited(false);

            EXPECT_EQ(1, platform.TestPriorityToInt(V8TaskPriority::kBestEffort));
            EXPECT_EQ(2, platform.TestPriorityToInt(V8TaskPriority::kUserVisible));
            EXPECT_EQ(3, platform.TestPriorityToInt(V8TaskPriority::kUserBlocking));
            EXPECT_EQ(3, platform.TestPriorityToInt(V8TaskPriority::kMaxPriority));
        }
    } // namespace JSRuntime
} // namespace v8App