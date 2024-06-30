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
#include "V8AppPlatform.h"

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
            TestContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace, std::filesystem::path inEntryPoint,
                        std::filesystem::path inSnapEntryPoint = "", bool inSupportsSnapshot = true,
                        SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly) : JSContext(inRuntime, inName, inNamespace, inEntryPoint,
                                                                                                  inSnapEntryPoint, inSupportsSnapshot, inSnapMethod) {}
            void TestDisposeContext() { m_Runtime = nullptr; }
        };

        class TestContextCreator : public JSContextCreationHelper
        {
        public:
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespce,
                                                     std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint,
                                                     bool inSupportsSnapshot, SnapshotMethod inSnapMethod) override
            {
                return m_Context;
            }
            virtual void DisposeContext(JSContextSharedPtr inContext) override { m_Context->TestDisposeContext(); }
            virtual void RegisterSnapshotCloser(JSContextSharedPtr inContext) override {}
            virtual void UnregisterSnapshotCloser(JSContextSharedPtr inContext) override {}

            std::shared_ptr<TestContext> m_Context;
        };

        class TestCloseHandlerJSRuntime final : public ISnapshotHandleCloser
        {
        public:
            int m_Value = 0;

        protected:
            virtual void CloseHandleForSnapshot() { m_Value = 10; }
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
            EXPECT_EQ(1, *runtime->GetCppHeapID());
            EXPECT_EQ(nullptr, runtime->GetCppHeap());
            EXPECT_EQ(nullptr, runtime->GetContextCreationHelper());

            runtime->DisposeRuntime();
            EXPECT_EQ(nullptr, runtime->GetIsolate());
            EXPECT_EQ(nullptr, runtime->GetSharedIsolate());
            EXPECT_EQ(nullptr, runtime->GetForegroundTaskRunner());
            EXPECT_EQ(nullptr, runtime->GetApp());
            EXPECT_EQ(nullptr, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
        }

        TEST_F(JSRuntimeTest, CreateJSRuntime)
        {
            JSContextCreationHelperSharedPtr contextCreator = std::make_shared<TestContextCreator>();
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksEnabled, "testCreateJSRutimeIdleEnabled", contextCreator);
            EXPECT_TRUE(runtimePtr->IdleTasksEnabled());
            EXPECT_NE(runtimePtr->GetSharedIsolate().get(), nullptr);
            EXPECT_NE(runtimePtr->GetIsolate(), nullptr);
            EXPECT_EQ(runtimePtr, JSRuntime::GetJSRuntimeFromV8Isolate(runtimePtr->GetIsolate()));
            EXPECT_EQ(contextCreator, runtimePtr->GetContextCreationHelper());
            EXPECT_NE(nullptr, runtimePtr->GetCppHeap());
            runtimePtr->DisposeRuntime();
            runtimePtr.reset();

            runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksDisabled, "testCreateJSRutimeIdleNotEnabled", contextCreator);
            EXPECT_FALSE(runtimePtr->IdleTasksEnabled());
            runtimePtr->DisposeRuntime();
            runtimePtr.reset();
        }

        TEST_F(JSRuntimeTest, CreateJSRuntimeForSnapshot)
        {
            JSContextCreationHelperSharedPtr contextCreator = std::make_shared<TestContextCreator>();
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksEnabled, "testCreateJSRuntimeForSnapshotIdleEnabled", contextCreator, true);
            EXPECT_TRUE(runtimePtr->IdleTasksEnabled());
            EXPECT_NE(runtimePtr->GetSharedIsolate().get(), nullptr);
            EXPECT_NE(runtimePtr->GetSnapshotCreator().get(), nullptr);
            EXPECT_NE(runtimePtr->GetIsolate(), nullptr);
            EXPECT_EQ(contextCreator, runtimePtr->GetContextCreationHelper());
            EXPECT_EQ(runtimePtr, JSRuntime::GetJSRuntimeFromV8Isolate(runtimePtr->GetIsolate()));

            runtimePtr->DisposeRuntime();
            runtimePtr.reset();

            runtimePtr = JSRuntime::CreateJSRuntime(m_App, IdleTasksSupport::kIdleTasksDisabled, "testCreateJSRuntimeForSnapshotIdleNotEnabled", contextCreator, true);
            EXPECT_FALSE(runtimePtr->IdleTasksEnabled());

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
            JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, "ProcessTaskIdleTask", nullptr);
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
            V8IsolateScope isolateScope(runtimePtr->GetIsolate());
            V8HandleScope scope(runtimePtr->GetIsolate());

            V8LObjTpl objTemplate = V8ObjTpl::New(runtimePtr->GetIsolate());
            EXPECT_FALSE(objTemplate.IsEmpty());

            struct TemplateInfo info;

            EXPECT_TRUE(runtimePtr->GetObjectTemplate(&info).IsEmpty());

            runtimePtr->SetObjectTemplate(&info, objTemplate);
            EXPECT_FALSE(runtimePtr->GetObjectTemplate(&info).IsEmpty());
        }

        TEST_F(JSRuntimeTest, GetSetContextCreationHelper)
        {
            JSRuntimeSharedPtr runtime = m_App->GetJSRuntime();
            EXPECT_NE(nullptr, runtime->GetContextCreationHelper());
            std::shared_ptr<TestContextCreator> creator;
            runtime->SetContextCreationHelper(creator);
            EXPECT_EQ(nullptr, runtime->GetContextCreationHelper());
        }

        TEST_F(JSRuntimeTest, AddGetSnapNamespaces)
        {
            JSRuntimeSharedPtr runtime = m_App->GetJSRuntime();
            EXPECT_EQ(JSRuntime::kMaxContextNamespaces, runtime->GetSnapIndexForNamespace("test"));
            EXPECT_EQ("", runtime->GetNamespaceForSnapIndex(0));
            EXPECT_TRUE(runtime->AddSnapIndexNamespace(0, "test"));
            EXPECT_EQ(0, runtime->GetSnapIndexForNamespace("test"));
            EXPECT_EQ("test", runtime->GetNamespaceForSnapIndex(0));
            EXPECT_FALSE(runtime->AddSnapIndexNamespace(0, "test1"));
            EXPECT_FALSE(runtime->AddSnapIndexNamespace(1, "test"));
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
            V8IsolateScope isolateScope(runtimePtr->GetIsolate());

            std::shared_ptr<TestContextCreator> helper = std::make_shared<TestContextCreator>();

            runtimePtr->SetContextCreationHelper(helper);

            EXPECT_EQ(nullptr, runtimePtr->GetContextByName("test"));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Failed to find JSContext with name test"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            EXPECT_EQ(nullptr, runtimePtr->CreateContext("test", ""));

            expected = {
                {Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            logSink->FlushMessages();

            helper->m_Context = std::make_shared<TestContext>(runtimePtr, "test", "", "");

            EXPECT_NE(nullptr, runtimePtr->CreateContext("test", ""));
            EXPECT_TRUE(logSink->NoMessages());
            EXPECT_NE(nullptr, runtimePtr->GetContextByName("test"));
            helper->DisposeContext(std::shared_ptr<TestContext>());
        }

        TEST_F(JSRuntimeTest, RegisterUnregisterCloseHandlers)
        {
            JSAppSharedPtr snapApp = m_App->CreateSnapshotApp();
            JSRuntimeSharedPtr runtime = snapApp->GetJSRuntime();
            std::shared_ptr<TestCloseHandlerJSRuntime> closer = std::make_shared<TestCloseHandlerJSRuntime>();

            runtime->RegisterSnapshotHandleCloser(closer);
            EXPECT_EQ(closer->m_Value, 0);
            runtime->CloseOpenHandlesForSnapshot();
            EXPECT_EQ(closer->m_Value, 10);
            closer->m_Value = 0;
            EXPECT_EQ(closer->m_Value, 0);
            runtime->UnregisterSnapshotHandlerCloser(closer);
            runtime->CloseOpenHandlesForSnapshot();
            EXPECT_EQ(closer->m_Value, 0);
            snapApp->DisposeApp();
        }

    }
}