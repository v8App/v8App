// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestLogSink.h"
#include "Utils/Environment.h"
#include "Utils/Format.h"

#include "JSRuntime.h"
#include "V8Platform.h"

#include "V8InitApp.h"
#include "test_main.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSRuntimeTest = V8InitApp;

        struct TemplateInfo
        {
            bool m_Bool;
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

            void Run(double deadline) override
            {
                std::this_thread::sleep_for(std::chrono::seconds(m_Sleep));
                *m_Int = m_Value;
            }

        protected:
            int *m_Int;
            int m_Value;
            int m_Sleep;
        };

        class TestContext : public JSContext
        {
        public:
            TestContext(JSRuntimeSharedPtr inRuntime, std::string inName) : JSContext(inRuntime, inName) {}
            void TestDisposeContext() { m_Runtime = nullptr; }
        };

        class TestContextCreator : public JSContextCreationHelper
        {
        public:
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName) override
            {
                return m_Context;
            }
            virtual void DisposeContext(JSContextSharedPtr inContext) override { m_Context->TestDisposeContext(); }

            std::shared_ptr<TestContext> m_Context;
        };

        TEST_F(JSRuntimeTest, Constrcutor)
        {
            std::string runtimeName = "testJSRuntimeConstructor";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTasksSupport::kIdleTasksEnabled, runtimeName);
            ASSERT_NE(nullptr, runtime);
            EXPECT_EQ(true, runtime->IdleTasksEnabled());
            EXPECT_EQ(nullptr, runtime->GetIsolate());
            EXPECT_EQ(nullptr, runtime->GetSharedIsolate());
            EXPECT_EQ(nullptr, runtime->GetSnapshotCreator());
            EXPECT_EQ(runtimeName, runtime->GetName());
            EXPECT_NE(runtime->GetForegroundTaskRunner().get(), nullptr);
            EXPECT_EQ(nullptr, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
            EXPECT_EQ(m_App, runtime->GetApp());

            runtime->DisposeRuntime();
            EXPECT_EQ(nullptr, runtime->GetIsolate());
            EXPECT_EQ(nullptr, runtime->GetSharedIsolate());
            EXPECT_EQ(nullptr, runtime->GetForegroundTaskRunner());
            EXPECT_EQ(nullptr, runtime->GetApp());
            EXPECT_EQ(nullptr, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
        }

        TEST_F(JSRuntimeTest, CreateJSRuntime)
        {
            std::vector<intptr_t> testExternal{reinterpret_cast<intptr_t>(nullptr)};
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksEnabled, "testCreateJSRutimeIdleEnabled", &s_V8StartupData, testExternal.data());
            EXPECT_TRUE(runtimePtr->IdleTasksEnabled());
            EXPECT_NE(runtimePtr->GetSharedIsolate().get(), nullptr);
            EXPECT_NE(runtimePtr->GetIsolate(), nullptr);
            EXPECT_EQ(runtimePtr, JSRuntime::GetJSRuntimeFromV8Isolate(runtimePtr->GetIsolate()));
            runtimePtr->DisposeRuntime();
            runtimePtr.reset();

            runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksDisabled, "testCreateJSRutimeIdleNotEnabled", &s_V8StartupData);
            EXPECT_FALSE(runtimePtr->IdleTasksEnabled());
            runtimePtr->DisposeRuntime();
            runtimePtr.reset();
        }

        TEST_F(JSRuntimeTest, CreateJSRuntimeForSnapshot)
        {

            std::vector<intptr_t> testExternal{reinterpret_cast<intptr_t>(nullptr)};
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksEnabled, "testCreateJSRuntimeForSnapshotIdleEnabled", &s_V8StartupData, testExternal.data(), true);
            EXPECT_TRUE(runtimePtr->IdleTasksEnabled());
            EXPECT_NE(runtimePtr->GetSharedIsolate().get(), nullptr);
            EXPECT_NE(runtimePtr->GetSnapshotCreator().get(), nullptr);
            EXPECT_NE(runtimePtr->GetIsolate(), nullptr);
            EXPECT_EQ(runtimePtr, JSRuntime::GetJSRuntimeFromV8Isolate(runtimePtr->GetIsolate()));

            // we need to init the isolate before we dispoe of it
            // v8::Isolate::CreateParams params;
            // params.snapshot_blob = &s_V8StartupData;
            // params.array_buffer_allocator =
            //    v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            // v8::Isolate::Initialize(runtimePtr->GetIsolate(), params);

            runtimePtr->DisposeRuntime();
            runtimePtr.reset();

            runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksDisabled, "testCreateJSRuntimeForSnapshotIdleNotEnabled", &s_V8StartupData, testExternal.data(), true);
            EXPECT_FALSE(runtimePtr->IdleTasksEnabled());

            // we need to init the isolate before we dispoe of it
            // params.snapshot_blob = &s_V8StartupData;
            // params.array_buffer_allocator =
            //    v8::ArrayBuffer::Allocator::NewDefaultAllocator();
            // v8::Isolate::Initialize(runtimePtr->GetIsolate(), params);

            runtimePtr->DisposeRuntime();
            runtimePtr.reset();
        }

        TEST_F(JSRuntimeTest, ProcessTaskIdleTask)
        {
            // make sure the time function is clear of any testing
            TestTime::TestTimeSeconds::Clear();
            int taskInt = 0;
            int taskInt2 = 0;
            int idleTaskInt = 0;
            int idleTaskInt2 = 0;

            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            JSAppSharedPtr app = std::make_shared<JSApp>("ProcessTaskIdleTask", snapProvider);
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, GENERATE_JSRUNTIME_NAME(), &s_V8StartupData);
            V8TaskRunnerSharedPtr runner = runtimePtr->GetForegroundTaskRunner();
            runner->PostTask(std::make_unique<IntTask>(&taskInt, 10));
            runner->PostTask(std::make_unique<IntTask>(&taskInt2, 20));
            runner->PostIdleTask(std::make_unique<IntIdleTask>(&idleTaskInt, 30, 3));
            runner->PostIdleTask(std::make_unique<IntIdleTask>(&idleTaskInt2, 40, 0));

            EXPECT_EQ(0, taskInt);
            EXPECT_EQ(0, taskInt2);
            EXPECT_EQ(0, idleTaskInt);
            EXPECT_EQ(0, idleTaskInt2);

            runtimePtr->ProcessTasks();
            EXPECT_EQ(10, taskInt);
            EXPECT_EQ(20, taskInt2);
            EXPECT_EQ(0, idleTaskInt);
            EXPECT_EQ(0, idleTaskInt2);

            runtimePtr->ProcessIdleTasks(1);
            EXPECT_EQ(30, idleTaskInt);
            EXPECT_EQ(0, idleTaskInt2);

            runtimePtr->ProcessIdleTasks(5);
            EXPECT_EQ(30, idleTaskInt);
            EXPECT_EQ(40, idleTaskInt2);
        }

        TEST_F(JSRuntimeTest, SetGetObjectTemplate)
        {
            JSRuntimeSharedPtr runtimePtr = m_App->GetJSRuntime();
            v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate());
            v8::HandleScope scope(runtimePtr->GetIsolate());

            v8::Local<v8::ObjectTemplate> objTemplate = v8::ObjectTemplate::New(runtimePtr->GetIsolate());
            EXPECT_FALSE(objTemplate.IsEmpty());

            struct TemplateInfo info;

            EXPECT_TRUE(runtimePtr->GetObjectTemplate(&info).IsEmpty());

            runtimePtr->SetObjectTemplate(&info, objTemplate);
            EXPECT_FALSE(runtimePtr->GetObjectTemplate(&info).IsEmpty());
        }

        TEST_F(JSRuntimeTest, SetGetFunctionTemplate)
        {
            JSRuntimeSharedPtr runtimePtr = m_App->GetJSRuntime();
            v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate());
            v8::HandleScope scope(runtimePtr->GetIsolate());

            v8::Local<v8::FunctionTemplate> funcTemplate = v8::FunctionTemplate::New(runtimePtr->GetIsolate());
            ASSERT_FALSE(funcTemplate.IsEmpty());

            struct TemplateInfo info;

            EXPECT_TRUE(runtimePtr->GetFunctionTemplate(&info).IsEmpty());

            runtimePtr->SetFunctionTemplate(&info, funcTemplate);
            EXPECT_FALSE(runtimePtr->GetFunctionTemplate(&info).IsEmpty());
        }

        TEST_F(JSRuntimeTest, GetCreateContext)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);
            // make sure no message are in the list
                logSink->FlushMessages();

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};
            Log::Log::SetLogLevel(Log::LogLevel::Warn);

            JSRuntimeSharedPtr runtimePtr = m_App->GetJSRuntime();
            v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate());

            std::unique_ptr<TestContextCreator> helper = std::make_unique<TestContextCreator>();
            TestContextCreator *createPtr = helper.get();

            runtimePtr->SetContextCreationHelper(std::move(helper));

            EXPECT_EQ(nullptr, runtimePtr->GetContextByName("test"));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Failed to find JSContext with name test"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            EXPECT_EQ(nullptr, runtimePtr->CreateContext("test"));

            expected = {
                {Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            logSink->FlushMessages();

            createPtr->m_Context = std::make_shared<TestContext>(runtimePtr, "test");

            EXPECT_NE(nullptr, runtimePtr->CreateContext("test"));
            EXPECT_TRUE(logSink->NoMessages());
            EXPECT_NE(nullptr, runtimePtr->GetContextByName("test"));
            createPtr->DisposeContext(std::shared_ptr<TestContext>());
        }

        TEST_F(JSRuntimeTest, GenerateRuntimeName)
        {
            EXPECT_EQ(Utils::format("JSRuntime_{}_{}_{}", __FILE__, __func__, __LINE__), GENERATE_JSRUNTIME_NAME());
        }
    }
}