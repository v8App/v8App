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
                        size_t inSnapIndex, bool inSupportsSnapshot = true,
                        SnapshotMethod inSnapMethod = SnapshotMethod::kNamespaceOnly) : JSContext(inRuntime, inName, inNamespace, inEntryPoint, inSnapIndex,
                                                                                                  inSupportsSnapshot, inSnapMethod) {}
            void TestDisposeContext() { m_Runtime = nullptr; }
        };

        class TestContextCreator : public IJSContextProvider
        {
        public:
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespce,
                                                     std::filesystem::path inEntryPoint,
                                                     bool inSupportsSnapshot, SnapshotMethod inSnapMethod, size_t inContextIndex) override
            {
                return m_Context;
            }
            virtual void DisposeContext(JSContextSharedPtr inContext) override { m_Context->TestDisposeContext(); }

            std::shared_ptr<TestContext> m_Context;
        };

        class TestJSRuntimeCloseHandler final : public ISnapshotHandleCloser, std::enable_shared_from_this<TestJSRuntimeCloseHandler>
        {
        public:
            int m_Value = 0;

        protected:
            virtual void CloseHandleForSnapshot() { m_Value = 10; }
        };

        class TestCloseHandlerJSRuntime : public JSRuntime
        {
        public:
            TestCloseHandlerJSRuntime() {}
            virtual ~TestCloseHandlerJSRuntime() {};

            int GetNumberOfClosers() { return m_HandleClosers.size(); }
            void SetSnapshotter(bool inValue) { m_IsSnapshotter = inValue; }
        };

        TEST_F(JSRuntimeTest, Constrcutor)
        {
            std::string runtimeName = "testJSRuntimeConstructor";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();

            EXPECT_EQ(runtime->GetForegroundTaskRunner(), nullptr);
            EXPECT_EQ(false, runtime->IdleTasksEnabled());
            EXPECT_EQ(nullptr, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
            EXPECT_EQ(nullptr, runtime->GetSharedIsolate());
            EXPECT_EQ(nullptr, runtime->GetIsolate());
            EXPECT_EQ(nullptr, runtime->GetApp());
            EXPECT_EQ("", runtime->GetName());
            EXPECT_EQ(nullptr, runtime->GetSnapshotCreator());
            EXPECT_FALSE(runtime->IsSnapshotRuntime());
            EXPECT_EQ(nullptr, runtime->GetCppHeap());
            EXPECT_EQ(1, *runtime->GetCppHeapID());
            EXPECT_EQ(nullptr, runtime->GetContextProvider());
            EXPECT_FALSE(runtime->IsInitialzed());
        }

        TEST_F(JSRuntimeTest, InitializeDispose)
        {
            std::string runtimeName = "testJSRuntimeInitializeDispose";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();

            EXPECT_TRUE(runtime->Initialize(m_App, runtimeName));
            EXPECT_EQ(runtime, JSRuntime::GetJSRuntimeFromV8Isolate(runtime->GetIsolate()));
            EXPECT_EQ(m_App, runtime->GetApp());
            EXPECT_EQ(runtimeName, runtime->GetName());
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

            runtime = std::make_shared<JSRuntime>();
            EXPECT_FALSE(runtime->IdleTasksEnabled());
            runtime->DisposeRuntime();
            runtime.reset();
        }

        TEST_F(JSRuntimeTest, CreateJSRuntimeForSnapshot)
        {
            std::string runtimeName = "testJSRuntimeCreateJSRuntimeForSnapshot";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();
            ASSERT_TRUE(runtime->Initialize(m_App, runtimeName, 0, JSRuntimeSnapshotAttributes::SnapshotOnly, true));

            EXPECT_NE(runtime->GetSharedIsolate().get(), nullptr);
            EXPECT_NE(runtime->GetSnapshotCreator().get(), nullptr);
            EXPECT_NE(runtime->GetIsolate(), nullptr);
            EXPECT_TRUE(runtime->IsSnapshotRuntime());

            runtime->DisposeRuntime();
            EXPECT_EQ(runtime->GetSnapshotCreator().get(), nullptr);
            runtime.reset();

            runtime = std::make_shared<JSRuntime>();
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
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();
            ASSERT_TRUE(runtime->Initialize(m_App, runtimeName));

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

        TEST_F(JSRuntimeTest, SetGetClassFunctionTemplate)
        {
            std::string runtimeName = "testJSRuntimeSetGetClassFunctionTemplate";
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();
            ASSERT_TRUE(runtime->Initialize(m_App, runtimeName));

            V8IsolateScope isolateScope(runtime->GetIsolate());
            V8HandleScope scope(runtime->GetIsolate());

            V8LFuncTpl objTemplate = V8FuncTpl::New(runtime->GetIsolate());
            EXPECT_FALSE(objTemplate.IsEmpty());

            struct CppBridge::V8CppObjInfo info("test", nullptr, nullptr);

            EXPECT_TRUE(runtime->GetClassFunctionTemplate(&info).IsEmpty());

            runtime->SetClassFunctionTemplate("test", &info, objTemplate);
            EXPECT_FALSE(runtime->GetClassFunctionTemplate(&info).IsEmpty());
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
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();

            std::shared_ptr<TestContextCreator> contextProvider = std::make_shared<TestContextCreator>();

            ASSERT_TRUE(runtime->Initialize(m_App, runtimeName));
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

        TEST_F(JSRuntimeTest, RegisterUnregisterRunCloseHandlers)
        {
            std::filesystem::path testRoot = s_TestDir / "RegisterUnregisterCloseHandlers";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            IJSSnapshotCreatorSharedPtr provider = std::make_shared<TestSnapshotCreator>();
            AppProviders providers(std::make_shared<TestSnapshotProvider>(),
                                   std::make_shared<V8RuntimeProvider>(),
                                   std::make_shared<V8ContextProvider>());

            JSAppSharedPtr snapApp = std::make_shared<JSApp>();
            ASSERT_TRUE(snapApp->Initialize("RegisterUnregisterCloseHandlers", testRoot, providers));
            std::shared_ptr<TestJSRuntimeCloseHandler> closer = std::make_shared<TestJSRuntimeCloseHandler>();
            std::shared_ptr<TestJSRuntimeCloseHandler> closer2 = std::make_shared<TestJSRuntimeCloseHandler>();

            // non snapshot runtime
            std::shared_ptr<TestCloseHandlerJSRuntime> runtime = std::make_shared<TestCloseHandlerJSRuntime>();

            // won't add on non snapshotter
            runtime->SetSnapshotter(false);
            runtime->RegisterSnapshotHandleCloser(closer.get());
            EXPECT_EQ(0, runtime->GetNumberOfClosers());

            // adds it
            runtime->SetSnapshotter(true);
            runtime->RegisterSnapshotHandleCloser(closer.get());
            EXPECT_EQ(1, runtime->GetNumberOfClosers());

            // test unregister is not called when a non snapshotter
            runtime->SetSnapshotter(false);
            runtime->UnregisterSnapshotHandlerCloser(closer.get());
            EXPECT_EQ(1, runtime->GetNumberOfClosers());

            // test that it just removes the one we want
            runtime->SetSnapshotter(true);
            runtime->RegisterSnapshotHandleCloser(closer2.get());
            EXPECT_EQ(2, runtime->GetNumberOfClosers());
            runtime->UnregisterSnapshotHandlerCloser(closer2.get());
            EXPECT_EQ(1, runtime->GetNumberOfClosers());

            // test that the handlers don't get run if it's a snapshot
            runtime->SetSnapshotter(false);
            runtime->CloseOpenHandlesForSnapshot();
            EXPECT_EQ(1, runtime->GetNumberOfClosers());

            runtime->SetSnapshotter(true);
            runtime->CloseOpenHandlesForSnapshot();
            EXPECT_EQ(0, runtime->GetNumberOfClosers());
            EXPECT_EQ(10, closer->m_Value);

            snapApp->DisposeApp();
        }

    }
}