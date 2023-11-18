// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "JSRuntime.h"
#include "V8Platform.h"
#include "V8TestFixture.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace v8App
{
    namespace JSRuntime
    {
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

        TEST(JSRuntimeTest, ConstrcutorRelated)
        {
            std::unique_ptr<Runfiles> runfiles = InitV8Platform();

            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(IdleTasksSupport::kIdleTasksDisabled);
            EXPECT_TRUE(runtimePtr->AreIdleTasksEnabled());
            EXPECT_NE(runtimePtr->m_TaskRunner.get(), nullptr);
            EXPECT_NE(runtimePtr->m_DelayedWorkerTasks.get(), nullptr);
            EXPECT_NE(runtimePtr->GetIsolate().get(), nullptr);
            EXPECT_EQ(runtimePtr->GetIsolate().get(), runtimePtr->m_Isolate.get());

            JSRuntimeSharedPtr runtimePtr2 = JSRuntime::CreateJSRuntime(IdleTasksSupport::kIdleTasksDisabled);
            EXPECT_FALSE(runtimePtr2->AreIdleTasksEnabled());
            EXPECT_NE(runtimePtr2->m_TaskRunner.get(), nullptr);
            EXPECT_NE(runtimePtr2->m_DelayedWorkerTasks.get(), nullptr);
            EXPECT_NE(runtimePtr2->GetIsolate().get(), nullptr);

            EXPECT_EQ(runtimePtr2.get(), JSRuntime::GetJSRuntimeFromV8Isolate(runtimePtr2->GetIsolate().get()).get());
        }

        TEST(JSRuntimeTest, CreateContext)
        {
            std::unique_ptr<Runfiles> runfiles = InitV8Platform();

            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(IdleTasksSupport::kIdleTasksDisabled);
            JSContextWeakPtr weakContext = runtimePtr->CreateContext("Test");
            JSContextSharedPtr sharedContext = weakContext.lock();
            EXPECT_EQ(sharedContext.get(), runtimePtr->GetContextByName("test").lock().get());
            
        }
    }
}