
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Platform.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestForegroundTaskRunner : public v8::TaskRunner
        {
        public:
            virtual void PostTask(std::unique_ptr<v8::Task> task) override {}

            virtual void PostDelayedTask(std::unique_ptr<v8::Task> task,
                                         double delay_in_seconds) override {}
            virtual void PostIdleTask(std::unique_ptr<v8::IdleTask> task) override {}
            virtual bool IdleTasksEnabled() override {return true;}
        };

        class TestIsolateHelper : public PlatformIsolateHelper
        {
        public:
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(v8::Isolate *inIsolate, v8::TaskPriority priority) override
            {
                return std::make_shared<TestForegroundTaskRunner>();
            };
            virtual bool IdleTasksEnabled(v8::Isolate *inIsolate) override { return true; }
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
            enum Type Type() const override { return v8::ThreadIsolatedAllocator::Type::kPkey; }
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

        class TestV8Platform : public V8Platform
        {
        public:
            TestV8Platform() {}

            Threads::ThreadPriority TestIntToPriority(int inInt) { return IntToPriority(inInt); }
            int TestPriorityToInt(v8::TaskPriority inPriority) { return PriorityToInt(inPriority); }
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

        TEST(V8PlatformTest, ConstructorDestructor)
        {
            int cores = Threads::GetHardwareCores();
            TestV8Platform platform;
            EXPECT_EQ(platform.NumberOfWorkerThreads(), cores);
            EXPECT_FALSE(platform.IsInited());
            EXPECT_EQ(platform.CreateBlockingScope(v8::BlockingType::kMayBlock), nullptr);
            EXPECT_EQ(platform.GetStackTracePrinter(), nullptr);
        }

        TEST(V8PlatformTest, GetSetTracingController)
        {
            TestV8Platform platform;
            v8::TracingController *constrcuted = platform.GetTracingController();
            EXPECT_NE(constrcuted, nullptr);

            platform.SetTracingController(nullptr);
            EXPECT_EQ(platform.GetTracingController(), constrcuted);

            platform.SetInited(true);
            platform.SetTracingController(nullptr);
            EXPECT_EQ(platform.GetTracingController(), constrcuted);

            std::unique_ptr<v8::TracingController> controller = std::make_unique<v8::TracingController>();
            platform.SetInited(false);
            platform.SetTracingController(controller.get());
            EXPECT_EQ(platform.GetTracingController(), constrcuted);

            platform.SetInited(true);
            platform.SetTracingController(controller.get());
            EXPECT_EQ(platform.GetTracingController(), controller.get());
            controller.release();
        }

        TEST(V8PlatformTest, GetSetPageAllocator)
        {
            TestV8Platform platform;
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

        TEST(V8PlatformTest, GetSetThreadIsolatedAllocator)
        {
            TestV8Platform platform;
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

        TEST(V8PlatformTest, GetSetZoneBackingAllocator)
        {
            TestV8Platform platform;
            v8::ZoneBackingAllocator *constrcuted = platform.GetZoneBackingAllocator();
            EXPECT_NE(constrcuted, nullptr);

            std::unique_ptr<v8::ZoneBackingAllocator> allocator = std::make_unique<v8::ZoneBackingAllocator>();
            platform.SetZoneBlockingAllocator(allocator.get());
            EXPECT_EQ(platform.GetZoneBackingAllocator(), constrcuted);

            platform.SetInited(true);
            platform.SetZoneBlockingAllocator(allocator.get());
            EXPECT_EQ(platform.GetZoneBackingAllocator(), allocator.get());

            platform.SetZoneBlockingAllocator(nullptr);
            EXPECT_EQ(platform.GetZoneBackingAllocator(), allocator.get());

            allocator.release();
        }

        TEST(V8PlatformTest, MonotonicallyIncreasingTime)
        {
            TestV8Platform platform;
            // Make sure we're using the normal time function
            TestTime::TestTimeSeconds::Clear();
            double current = Time::MonotonicallyIncreasingTimeSeconds();
            EXPECT_DOUBLE_EQ(platform.MonotonicallyIncreasingTime(), current);
        }

        TEST(V8PlatformTest, CurrentClockTimeMilliseconds)
        {
            TestV8Platform platform;
            // Make sure we're using the normal time function
            TestTime::TestTimeMilliSeconds::Clear();
            double current = Time::MonotonicallyIncreasingTimeMilliSeconds();
            EXPECT_DOUBLE_EQ(platform.CurrentClockTimeMilliseconds(), current);
        }

        TEST(V8PlatformTest, GetSetHighAllocationThroughputObserver)
        {
            TestV8Platform platform;
            v8::HighAllocationThroughputObserver *constrcuted = platform.GetHighAllocationThroughputObserver();
            EXPECT_NE(constrcuted, nullptr);

            platform.SetInited(false);
            std::unique_ptr<v8::HighAllocationThroughputObserver> observer = std::make_unique<v8::HighAllocationThroughputObserver>();
            platform.SetHighAllocatoionObserver(observer.get());
            EXPECT_EQ(platform.GetHighAllocationThroughputObserver(), constrcuted);

            platform.SetInited(true);
            platform.SetHighAllocatoionObserver(observer.get());
            EXPECT_EQ(platform.GetHighAllocationThroughputObserver(), observer.get());

            platform.SetHighAllocatoionObserver(nullptr);
            EXPECT_EQ(platform.GetHighAllocationThroughputObserver(), observer.get());

            observer.release();
        }

        TEST(V8PlatformTest, GetForegroundTaskRunner)
        {
            std::shared_ptr<V8Platform> platform = V8Platform::Get();
            PlatformIsolateHelperUniquePtr helper = std::make_unique<TestIsolateHelper>();
            platform->SetIsolateHelper(std::move(helper));
            EXPECT_NE(nullptr, platform->GetForegroundTaskRunner(nullptr, v8::TaskPriority::kBestEffort).get());
        }

        TEST(V8PlatformTest, IdleTasksEnabled)
        {
            std::shared_ptr<V8Platform> platform = V8Platform::Get();
            PlatformIsolateHelperUniquePtr helper = std::make_unique<TestIsolateHelper>();
            platform->SetIsolateHelper(std::move(helper));
            EXPECT_TRUE(platform->IdleTasksEnabled(nullptr));
        }

        TEST(V8PlatformTest, Get)
        {
            std::shared_ptr<V8Platform> platform = V8Platform::Get();
            EXPECT_NE(platform, nullptr);
            EXPECT_EQ(platform, V8Platform::Get());
        }

        TEST(V8PlatformTest, CreateJobImpl)
        {
            TestV8Platform platform;
            platform.SetInited(false);

            std::unique_ptr<v8::JobHandle> handle;
            std::unique_ptr<TestPlatformJobTask> task = std::make_unique<TestPlatformJobTask>();

            auto start = std::chrono::high_resolution_clock::now();
            handle = platform.PostJob(v8::TaskPriority::kBestEffort, std::move(task));
            handle->Join();
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            EXPECT_GE(elapsed, 2);

            start = std::chrono::high_resolution_clock::now();
            task = std::make_unique<TestPlatformJobTask>();
            handle = platform.CreateJob(v8::TaskPriority::kBestEffort, std::move(task));
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

        TEST(V8PlatformTest, PostTaskOnWorkerThreadImpl)
        {
            TestV8Platform platform;
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

        TEST(V8PlatformTest, PostDelayedTaskOnWorkerThreadImpl)
        {
            TestV8Platform platform;
            platform.SetInited(false);

            EXPECT_EQ(testPlatformInt2, 0);
            platform.CallDelayedOnWorkerThread(std::make_unique<TestPlatformTask>(&testPlatformInt2, 2), 10);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(testPlatformInt2, 0);
            std::this_thread::sleep_for(std::chrono::seconds(10));
            EXPECT_EQ(testPlatformInt2, 2);
        }

        TEST(V8PlatformTest, IntToPriority)
        {
            TestV8Platform platform;
            platform.SetInited(false);

            int max = static_cast<int>(Threads::ThreadPriority::kMaxPriority) + 1;

            EXPECT_EQ(Threads::ThreadPriority::kBestEffort, platform.TestIntToPriority(-1));
            EXPECT_EQ(Threads::ThreadPriority::kDefault, platform.TestIntToPriority(0));
            EXPECT_EQ(Threads::ThreadPriority::kBestEffort, platform.TestIntToPriority(1));
            EXPECT_EQ(Threads::ThreadPriority::kUserVisible, platform.TestIntToPriority(2));
            EXPECT_EQ(Threads::ThreadPriority::kUserBlocking, platform.TestIntToPriority(3));
            EXPECT_EQ(Threads::ThreadPriority::kBestEffort, platform.TestIntToPriority(max));
        }

        TEST(V8PlatformTest, PriorityToInt)
        {
            TestV8Platform platform;
            platform.SetInited(false);

            EXPECT_EQ(1, platform.TestPriorityToInt(v8::TaskPriority::kBestEffort));
            EXPECT_EQ(2, platform.TestPriorityToInt(v8::TaskPriority::kUserVisible));
            EXPECT_EQ(3, platform.TestPriorityToInt(v8::TaskPriority::kUserBlocking));
            EXPECT_EQ(3, platform.TestPriorityToInt(v8::TaskPriority::kMaxPriority));
        }
    } // namespace JSRuntime
} // namespace v8App