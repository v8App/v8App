// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestLogSink.h"
#include "V8InitApp.h"
#include "test_main.h"
#include "TestSnapshotCreator.h"
#include "TestSnapshotProvider.h"
#include "TestFiles.h"

#include "Utils/Environment.h"
#include "Utils/Format.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8AppPlatform.h"

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
                        size_t inSnapIndex, std::filesystem::path inSnapEntryPoint = "", bool inSupportsSnapshot = true,
                        SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly) : JSContext(inRuntime, inName, inNamespace, inEntryPoint, inSnapIndex,
                                                                                                  inSnapEntryPoint, inSupportsSnapshot, inSnapMethod) {}
            void TestDisposeContext() { m_Runtime = nullptr; }
        };

        class TestContextCreator : public IJSContextProvider
        {
        public:
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespce,
                                                     std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint,
                                                     bool inSupportsSnapshot, SnapshotMethod inSnapMethod, size_t inContextIndex) override
            {
                return m_Context;
            }
            virtual void DisposeContext(JSContextSharedPtr inContext) override { m_Context->TestDisposeContext(); }

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
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, runtimeName, false, 0);

            EXPECT_NE(runtime->GetForegroundTaskRunner().get(), nullptr);
            EXPECT_EQ(true, runtime->IdleTasksEnabled());
            EXPECT_EQ(nullptr, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
            EXPECT_EQ(nullptr, runtime->GetSharedIsolate());
            EXPECT_EQ(nullptr, runtime->GetIsolate());
            EXPECT_EQ(m_App, runtime->GetApp());
            EXPECT_EQ(runtimeName, runtime->GetName());
            EXPECT_EQ(nullptr, runtime->GetSnapshotCreator());
            EXPECT_FALSE(runtime->IsSnapshotRuntime());
            EXPECT_EQ(nullptr, runtime->GetCppHeap());
            EXPECT_EQ(1, *runtime->GetCppHeapID());
            EXPECT_NE(nullptr, runtime->GetContextProvider());
            EXPECT_FALSE(runtime->IsInitialzed());

            runtime->DisposeRuntime();
            EXPECT_EQ(nullptr, runtime->GetForegroundTaskRunner().get());
            EXPECT_EQ(nullptr, runtime->GetApp());
        }

        TEST_F(JSRuntimeTest, InitializeDispose)
        {
            std::string runtimeName = "testJSRuntimeInitializeDispose";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, runtimeName, false, 0);

            EXPECT_TRUE(runtime->Initialize());
            EXPECT_EQ(runtime, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
            EXPECT_NE(nullptr, runtime->GetSharedIsolate());
            EXPECT_NE(nullptr, runtime->GetIsolate());
            EXPECT_EQ(nullptr, runtime->GetSnapshotCreator());
            EXPECT_FALSE(runtime->IsSnapshotRuntime());
            EXPECT_NE(nullptr, runtime->GetCppHeap());
            EXPECT_EQ(1, *runtime->GetCppHeapID());
            EXPECT_EQ(m_App->GetContextProvider(), runtime->GetContextProvider());
            EXPECT_TRUE(runtime->IsInitialzed());

            runtime->DisposeRuntime();
            EXPECT_EQ(nullptr, runtime->GetForegroundTaskRunner().get());
            EXPECT_EQ(nullptr, runtime->GetSharedIsolate());
            EXPECT_EQ(nullptr, runtime->GetIsolate());
            EXPECT_EQ(nullptr, runtime->GetApp());
            EXPECT_EQ(nullptr, runtime->GetSnapshotCreator());
            EXPECT_EQ(nullptr, runtime->GetCppHeap());
            EXPECT_EQ(nullptr, runtime->GetContextProvider());
            EXPECT_FALSE(runtime->IsInitialzed());
            runtime.reset();

            runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kDisabled, runtimeName, false, 0);
            EXPECT_FALSE(runtime->IdleTasksEnabled());
            runtime->DisposeRuntime();
            runtime.reset();
        }

        TEST_F(JSRuntimeTest, CreateJSRuntimeForSnapshot)
        {
            std::string runtimeName = "testJSRuntimeCreateJSRuntimeForSnapshot";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, runtimeName, true, 0);
            ASSERT_TRUE(runtime->Initialize());

            EXPECT_NE(runtime->GetSharedIsolate().get(), nullptr);
            EXPECT_NE(runtime->GetSnapshotCreator().get(), nullptr);
            EXPECT_NE(runtime->GetIsolate(), nullptr);
            EXPECT_TRUE(runtime->IsSnapshotRuntime());

            runtime->DisposeRuntime();
            EXPECT_EQ(runtime->GetSnapshotCreator().get(), nullptr);
            runtime.reset();

            runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kDisabled, runtimeName, true, 0);
            EXPECT_FALSE(runtime->IdleTasksEnabled());

            runtime->DisposeRuntime();
            runtime.reset();
        }

        TEST_F(JSRuntimeTest, ProcessTaskIdleTask)
        {
            // make sure the time function is clear of any testing
            TestTime::TestTimeSeconds::Clear();
            int taskInt = 0;
            int taskInt2 = 0;
            int idleTaskInt = 0;
            int idleTaskInt2 = 0;

            std::string runtimeName = "testJSRuntimeProcessTaskIdleTask";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, runtimeName, false, 0);
            ASSERT_TRUE(runtime->Initialize());

            V8TaskRunnerSharedPtr runner = runtime->GetForegroundTaskRunner();
            runner->PostTask(std::make_unique<IntTask>(&taskInt, 10));
            runner->PostTask(std::make_unique<IntTask>(&taskInt2, 20));
            runner->PostIdleTask(std::make_unique<IntIdleTask>(&idleTaskInt, 30, 3));
            runner->PostIdleTask(std::make_unique<IntIdleTask>(&idleTaskInt2, 40, 0));

            EXPECT_EQ(0, taskInt);
            EXPECT_EQ(0, taskInt2);
            EXPECT_EQ(0, idleTaskInt);
            EXPECT_EQ(0, idleTaskInt2);

            runtime->ProcessTasks();
            EXPECT_EQ(10, taskInt);
            EXPECT_EQ(20, taskInt2);
            EXPECT_EQ(0, idleTaskInt);
            EXPECT_EQ(0, idleTaskInt2);

            runtime->ProcessIdleTasks(1);
            EXPECT_EQ(30, idleTaskInt);
            EXPECT_EQ(0, idleTaskInt2);

            runtime->ProcessIdleTasks(5);
            EXPECT_EQ(30, idleTaskInt);
            EXPECT_EQ(40, idleTaskInt2);
        }

        TEST_F(JSRuntimeTest, SetGetObjectTemplate)
        {
            std::string runtimeName = "testJSRuntimeSetGetObjectTemplate";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, runtimeName, false, 0);
            ASSERT_TRUE(runtime->Initialize());

            V8IsolateScope isolateScope(runtime->GetIsolate());
            V8HandleScope scope(runtime->GetIsolate());

            V8LObjTpl objTemplate = V8ObjTpl::New(runtime->GetIsolate());
            EXPECT_FALSE(objTemplate.IsEmpty());

            struct TemplateInfo info;

            EXPECT_TRUE(runtime->GetObjectTemplate(&info).IsEmpty());

            runtime->SetObjectTemplate(&info, objTemplate);
            EXPECT_FALSE(runtime->GetObjectTemplate(&info).IsEmpty());
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

            std::string runtimeName = "testJSRuntimeCreateJSRuntimeForSnapshot";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(m_App, IdleTaskSupport::kEnabled, runtimeName, false, 0);

            std::shared_ptr<TestContextCreator> contextProvider = std::make_shared<TestContextCreator>();

            ASSERT_TRUE(runtime->Initialize());
            runtime->SetContextProvider(contextProvider);

            EXPECT_EQ(nullptr, runtime->GetContextByName("test"));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Failed to find JSContext with name test"},
                {Log::MsgKey::LogLevel, "Warn"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            EXPECT_EQ(nullptr, runtime->CreateContext("test", ""));

            expected = {
                {Log::MsgKey::Msg, "ContextHelper returned a nullptr for the context"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));
            logSink->FlushMessages();

            contextProvider->m_Context = std::make_shared<TestContext>(runtime, "test", "", "", 0);

            EXPECT_EQ(contextProvider->m_Context, runtime->CreateContext("test", ""));
            EXPECT_TRUE(logSink->NoMessages());
            EXPECT_EQ(contextProvider->m_Context, runtime->GetContextByName("test"));
            contextProvider->DisposeContext(std::shared_ptr<TestContext>());
            runtime->DisposeRuntime();
        }

        TEST_F(JSRuntimeTest, RegisterUnregisterCloseHandlers)
        {
            std::filesystem::path testRoot = s_TestDir / "RegisterUnregisterCloseHandlers";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            IJSSnapshotCreatorSharedPtr provider = std::make_shared<TestSnapshotCreator>();
            AppProviders providers(std::make_shared<TestSnapshotProvider>(),
                                   std::make_shared<V8RuntimeProvider>(),
                                   std::make_shared<V8ContextProvider>());

            JSAppSharedPtr snapApp = std::make_shared<JSApp>("RegisterUnregisterCloseHandlers", providers);
            ASSERT_TRUE(snapApp->Initialize(testRoot));

            JSRuntimeSharedPtr runtime = snapApp->GetMainRuntime();
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