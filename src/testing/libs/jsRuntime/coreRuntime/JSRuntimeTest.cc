// Copyright 2020 The v8App Authors. All rights reserved.
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

        TEST_F(JSRuntimeTest, CreateJSRuntime)
        {
            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, GENERATE_JSRUNTIME_NAME());
            EXPECT_TRUE(runtimePtr->IdleTasksEnabled());
            EXPECT_NE(runtimePtr->GetForegroundTaskRunner().get(), nullptr);
            EXPECT_NE(runtimePtr->GetIsolate().get(), nullptr);
            EXPECT_EQ(runtimePtr, JSRuntime::GetJSRuntimeFromV8Isolate(runtimePtr->GetIsolate().get()));
            runtimePtr.reset();

            runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksDisabled, GENERATE_JSRUNTIME_NAME());
            EXPECT_FALSE(runtimePtr->IdleTasksEnabled());
        }

        TEST_F(JSRuntimeTest, ProcessTaskIdleTask)
        {
            // make sure the time function is clear of any testing
            TestTime::TestTimeSeconds::Clear();
            int taskInt = 0;
            int taskInt2 = 0;
            int idleTaskInt = 0;
            int idleTaskInt2 = 0;

            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, GENERATE_JSRUNTIME_NAME());
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
            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, GENERATE_JSRUNTIME_NAME());
            v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
            v8::HandleScope scope(runtimePtr->GetIsolate().get());

            v8::Local<v8::ObjectTemplate> objTemplate = v8::ObjectTemplate::New(runtimePtr->GetIsolate().get());
            EXPECT_FALSE(objTemplate.IsEmpty());

            struct TemplateInfo info;

            EXPECT_TRUE(runtimePtr->GetObjectTemplate(&info).IsEmpty());

            runtimePtr->SetObjectTemplate(&info, objTemplate);
            EXPECT_FALSE(runtimePtr->GetObjectTemplate(&info).IsEmpty());
        }

        TEST_F(JSRuntimeTest, SetGetFunctionTemplate)
        {
            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, GENERATE_JSRUNTIME_NAME());
            v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
            v8::HandleScope scope(runtimePtr->GetIsolate().get());

            v8::Local<v8::FunctionTemplate> funcTemplate = v8::FunctionTemplate::New(runtimePtr->GetIsolate().get());
            ASSERT_FALSE(funcTemplate.IsEmpty());

            struct TemplateInfo info;

            EXPECT_TRUE(runtimePtr->GetFunctionTemplate(&info).IsEmpty());

            runtimePtr->SetFunctionTemplate(&info, funcTemplate);
            EXPECT_FALSE(runtimePtr->GetFunctionTemplate(&info).IsEmpty());
        }

        TEST_F(JSRuntimeTest, GetCreateContext)
        {
            TestUtils::WantsLogLevelsVector error = {Log::LogLevel::Warn, Log::LogLevel::Error};
            TestUtils::TestLogSink *logSink = new TestUtils::TestLogSink("TestLogSink", error);
            std::unique_ptr<Log::ILogSink> logSinkObj(logSink);
            EXPECT_TRUE(Log::Log::AddLogSink(logSinkObj));
            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};
            Log::Log::SetLogLevel(Log::LogLevel::Warn);

            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, GENERATE_JSRUNTIME_NAME());
            v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());

            std::unique_ptr<TestContextCreator> helper = std::make_unique<TestContextCreator>();
            TestContextCreator *createPtr = helper.get();

            runtimePtr->SetContextCreationHelper(std::move(helper));

            EXPECT_EQ(nullptr, runtimePtr->GetContextByName("test").lock());
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Failed to find JSContext with name test"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            logSink->FlushMessages();

            EXPECT_EQ(nullptr, runtimePtr->CreateContext("test").lock());

            expected = {
                {Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            logSink->FlushMessages();

            createPtr->m_Context = std::make_shared<TestContext>(runtimePtr, "test");

            EXPECT_NE(nullptr, runtimePtr->CreateContext("test").lock());
            EXPECT_TRUE(logSink->NoMessages());
            EXPECT_NE(nullptr, runtimePtr->GetContextByName("test").lock());
            createPtr->DisposeContext(std::shared_ptr<TestContext>());
        }

        TEST_F(JSRuntimeTest, GenerateRuntimeName)
        {
            EXPECT_EQ(Utils::format("JSRuntime_{}_{}_{}", __FILE__, __func__, __LINE__), GENERATE_JSRUNTIME_NAME());
        }
    }
}