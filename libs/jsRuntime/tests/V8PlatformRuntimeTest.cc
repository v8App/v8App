
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "third_party/bazel-runfiles/runfiles_src.h"
#include "v8.h"
#include "V8Platform.h"
#include "JSRuntime.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace v8App
{
    namespace JSRuntime
    {
        class DelayedWorkerTaskQueue;

        class MockJSRuntime : public JSRuntime
        {
        public:
            explicit MockJSRuntime(IdleTasksSupport inEnableIdle) : JSRuntime(inEnableIdle) {}
            DelayedWorkerTaskQueue *GetDelayedQueue() { return m_DelayedWorkerTasks.get(); }
        };

        class IntTask : public v8::Task
        {
        public:
            IntTask(int *inInt, int inValue) : m_Int(inInt), m_Value(inValue) {}
            virtual ~IntTask() {}

            void Run() override { *m_Int = m_Value; }

        protected:
            int *m_Int;
            int m_Value;
        };

        class IntIdleTask : public v8::IdleTask
        {
        public:
            IntIdleTask(int *inInt, int inValue, int inSleep) : m_Int(inInt), m_Value(inValue), m_Sleep(inSleep) {}
            virtual ~IntIdleTask() {}

            void Run(double inDeadline) override
            {
                *m_Int = m_Value;
                std::this_thread::sleep_for(std::chrono::seconds(m_Sleep));
            }

        protected:
            int *m_Int;
            int m_Value;
            int m_Sleep;
        };

        TEST(V8PlatformRuntimeTest, PlatformRuntime)
        {
            std::string error;
            std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
            if (error != "")
            {
                std::cout << error << std::endl;
            }
            ASSERT_NE(nullptr, runfiles);

            std::string icuData = runfiles->Rlocation("com_github_v8app_v8app/third_party/v8/*/icudtl.dat");
            std::string snapshotData = runfiles->Rlocation("com_github_v8app_v8app/third_party/v8/*/snapshot_blob.bin");

            ASSERT_NE("", icuData);
            ASSERT_NE("", snapshotData);

            //First test the innit of v8
            v8::V8::InitializeICU(icuData.c_str());
            v8::V8::InitializeExternalStartupDataFromFile(snapshotData.c_str());

            EXPECT_EQ(nullptr, V8Platform::Get().get());
            V8Platform::InitializeV8();
            EXPECT_NE(nullptr, V8Platform::Get().get());
            std::shared_ptr<V8Platform> platform = V8Platform::Get();
            V8Platform::InitializeV8();
            EXPECT_EQ(platform.get(), V8Platform::Get().get());

            //now we can create the runtime.
            std::unique_ptr<MockJSRuntime> runtime = std::make_unique<MockJSRuntime>(IdleTasksSupport::kIdleTasksEnabled);
            std::weak_ptr<v8::Isolate> isolate = runtime->GetIsolate();
            EXPECT_FALSE(isolate.expired());
            EXPECT_NE(nullptr, runtime->GetForegroundTaskRunner());
            EXPECT_NE(nullptr, runtime->GetDelayedQueue());
            EXPECT_EQ(runtime.get(), JSRuntime::GetRuntime(isolate.lock().get()));

            //now lets test the platofrm calls that require an isolate
            EXPECT_EQ(platform->GetForegroundTaskRunner(isolate.lock().get()).get(), runtime->GetForegroundTaskRunner().get());
            EXPECT_TRUE(platform->IdleTasksEnabled(isolate.lock().get()));

            //test running an task and idle task
            int taskInt = 0;
            int task2Int = 0;
            int idleInt = 0;
            int idle2Int = 0;

            TaskPtr task = std::make_unique<IntTask>(&taskInt, 5);
            runtime->GetForegroundTaskRunner()->PostTask(std::move(task));
            task = std::make_unique<IntTask>(&task2Int, 10);
            runtime->GetForegroundTaskRunner()->PostTask(std::move(task));

            IdleTaskPtr idleTask = std::make_unique<IntIdleTask>(&idleInt, 4, 3);
            runtime->GetForegroundTaskRunner()->PostIdleTask(std::move(idleTask));
            idleTask = std::make_unique<IntIdleTask>(&idleInt, 6, 1);
            runtime->GetForegroundTaskRunner()->PostIdleTask(std::move(idleTask));

            runtime->ProcessTasks();
            runtime->ProcessIdleTasks(2);

            EXPECT_EQ(5, taskInt);
            EXPECT_EQ(10, task2Int);
            EXPECT_EQ(4, idleInt);
            EXPECT_EQ(0, idle2Int);

            //test destructor
            runtime.reset();
            EXPECT_TRUE(isolate.expired());
            //and finally shutdown
            V8Platform::ShutdownV8();
        }
    } // namespace JSRuntime
} // namespace v8App