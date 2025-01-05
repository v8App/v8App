// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "TestLogSink.h"
#include "TestFiles.h"
#include "TestSnapshotProvider.h"
#include "TestSnapshotCreator.h"

#include "Assets/BinaryAsset.h"
#include "Assets/TextAsset.h"
#include "Utils/Paths.h"

#include "JSApp.h"
#include "JSContext.h"
#include "V8ContextProvider.h"
#include "V8ContextProvider.h"
#include "V8RuntimeProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class TestJSApp : public JSApp
        {
        public:
            TestJSApp() {}
            ~TestJSApp() {}

            void SetAppState(JSAppStates inState) { m_AppState = inState; }
        };

        TEST(JSAppTest, Constrcutor)
        {
            std::filesystem::path testRoot = s_TestDir / "ConstrcutorInitializeDispose";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::string appName = "testJSAppConstructor";
            JSAppSharedPtr app = std::make_shared<JSApp>();
            EXPECT_EQ("", app->GetName());
            EXPECT_EQ(nullptr, app->GetMainRuntime());
            EXPECT_EQ(nullptr, app->GetSnapshotCreator());
            EXPECT_EQ(nullptr, app->GetRuntimeProvider());
            EXPECT_EQ(nullptr, app->GetContextProvider());
            EXPECT_EQ(nullptr, app->GetSnapshotProvider());
            EXPECT_FALSE(app->IsSnapshotApp());
            EXPECT_EQ(nullptr, app->GetCodeCache());
            EXPECT_EQ(nullptr, app->GetAppRoot());
            EXPECT_FALSE(app->IsInitialized());
            EXPECT_EQ("0.0.0", app->GetAppVersion().GetVersionString());
        }

        TEST(JSAppTest, GetSetProviders)
        {
            std::string appName = "testJSAppGetSetProviders";

            std::filesystem::path testRoot = s_TestDir / "InitializeAsSnapshot";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            JSAppSharedPtr app = std::make_shared<JSApp>();

            EXPECT_EQ(nullptr, app->GetSnapshotCreator());
            EXPECT_EQ(nullptr, app->GetSnapshotProvider());
            EXPECT_EQ(nullptr, app->GetContextProvider());
            EXPECT_EQ(nullptr, app->GetRuntimeProvider());

            IJSSnapshotCreatorSharedPtr snapCreator = std::make_shared<TestSnapshotCreator>();
            IJSSnapshotProviderSharedPtr snapProvider = std::make_shared<TestSnapshotProvider>();
            IJSRuntimeProviderSharedPtr runtimeProvider = std::make_shared<V8RuntimeProvider>();
            IJSContextProviderSharedPtr contextProvider = std::make_shared<V8ContextProvider>();

            app->SetSnapshotCreator(snapCreator);
            app->SetSnapshotProvider(snapProvider);
            app->SetRuntimeProvider(runtimeProvider);
            app->SetContextProvider(contextProvider);

            EXPECT_EQ(snapCreator, app->GetSnapshotCreator());
            EXPECT_EQ(snapProvider, app->GetSnapshotProvider());
            EXPECT_EQ(contextProvider, app->GetContextProvider());
            EXPECT_EQ(runtimeProvider, app->GetRuntimeProvider());

            app->SetSnapshotCreator(nullptr);
            app->SetSnapshotProvider(nullptr);
            app->SetRuntimeProvider(nullptr);
            app->SetContextProvider(nullptr);

            EXPECT_EQ(snapCreator, app->GetSnapshotCreator());
            EXPECT_EQ(snapProvider, app->GetSnapshotProvider());
            EXPECT_EQ(contextProvider, app->GetContextProvider());
            EXPECT_EQ(runtimeProvider, app->GetRuntimeProvider());
        }

        TEST(JSAppTest, GetSetAppVersion)
        {
            std::shared_ptr<TestJSApp> app = std::make_shared<TestJSApp>();

            EXPECT_EQ("0.0.0", app->GetAppVersion().GetVersionString());
            app->SetAppVersion("test");
            EXPECT_EQ("0.0.0", app->GetAppVersion().GetVersionString());
            app->SetAppVersion("1.2.0");
            EXPECT_EQ("1.2.0", app->GetAppVersion().GetVersionString());

            Utils::VersionString version("2.2.2");
            app->SetAppVersion(version);
            EXPECT_EQ("2.2.2", app->GetAppVersion().GetVersionString());
        }

        TEST(JSAppTest, InitializeDispose)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::filesystem::path testRoot = s_TestDir / "InitializeDispose";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            std::string appName = "testJSAppConstructor";
            AppProviders providers;

            std::shared_ptr<TestJSApp> app = std::make_shared<TestJSApp>();
            std::shared_ptr snapProvider = std::make_shared<TestSnapshotProvider>();
            std::shared_ptr snapCreator = std::make_shared<TestSnapshotCreator>();
            // the creator isn't required to be set except during a snapshot
            providers.m_SnapshotCreator = snapCreator;

            // Tets app is restored
            app->SetAppState(JSAppStates::Restored);
            EXPECT_FALSE(app->Initialize(appName, testRoot, AppProviders()));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "Initalize can not be called on a restored app. Use RestoreInitialize."},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // test snapshot provider not set
            app->SetAppState(JSAppStates::Uninitialized);
            EXPECT_FALSE(app->Initialize(appName, testRoot, AppProviders()));
            expected = {
                {Log::MsgKey::Msg, "The snapshot provider must be set before calling Initialize"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // test runtime provider not set
            providers.m_SnapshotProvider = snapProvider;
            EXPECT_FALSE(app->Initialize(appName, testRoot, providers, false));
            expected = {
                {Log::MsgKey::Msg, "The runtime provider must be set before calling Initialize"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // test context provider not set
            providers.m_RuntimeProvider = std::make_shared<V8RuntimeProvider>();
            EXPECT_FALSE(app->Initialize(appName, testRoot, providers, false));
            expected = {
                {Log::MsgKey::Msg, "The context provider must be set before calling Initialize"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            providers.m_ContextProvider = std::make_shared<V8ContextProvider>();

            snapProvider->SetLoaded(false);
            EXPECT_FALSE(app->Initialize(appName, testRoot, providers, false));

            snapProvider->SetLoaded(true);
            snapProvider->SetReturnEmpty(true);
            EXPECT_FALSE(app->Initialize(appName, testRoot, providers, false));
            expected = {
                {Log::MsgKey::Msg, "Snapshot provider says data is loaded but default snapshot size is 0"},
                {Log::MsgKey::LogLevel, "Error"},
            };
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            snapProvider->SetReturnEmpty(false);
            EXPECT_TRUE(app->Initialize(appName, testRoot, providers, false));
            EXPECT_EQ(providers.m_SnapshotCreator, app->GetSnapshotCreator());
            EXPECT_EQ(providers.m_SnapshotProvider, app->GetSnapshotProvider());
            EXPECT_EQ(providers.m_ContextProvider, app->GetContextProvider());
            EXPECT_EQ(providers.m_RuntimeProvider, app->GetRuntimeProvider());
            EXPECT_NE(nullptr, app->GetMainRuntime());
            EXPECT_NE(nullptr, app->GetCodeCache());
            EXPECT_NE(nullptr, app->GetAppRoot());
            EXPECT_EQ(appName, app->GetName());
            EXPECT_TRUE(app->IsInitialized());
            EXPECT_FALSE(app->IsSnapshotApp());

            AppProviders newProviders;
            newProviders.m_SnapshotProvider = providers.m_SnapshotProvider;
            newProviders.m_RuntimeProvider = providers.m_RuntimeProvider;
            newProviders.m_ContextProvider = std::make_shared<V8ContextProvider>();

            EXPECT_TRUE(app->Initialize("NewAppName", testRoot, providers, false));
            EXPECT_EQ(providers.m_ContextProvider, app->GetContextProvider());
            EXPECT_EQ(appName, app->GetName());

            app->DisposeApp();
            EXPECT_EQ(nullptr, app->GetSnapshotCreator());
            EXPECT_EQ(nullptr, app->GetSnapshotProvider());
            EXPECT_EQ(nullptr, app->GetContextProvider());
            EXPECT_EQ(nullptr, app->GetRuntimeProvider());
            EXPECT_EQ(nullptr, app->GetMainRuntime());
            EXPECT_EQ(nullptr, app->GetCodeCache());
            EXPECT_EQ(nullptr, app->GetAppRoot());
            EXPECT_EQ(appName, app->GetName());
            EXPECT_FALSE(app->IsInitialized());
            EXPECT_FALSE(app->IsSnapshotApp());
        }

        TEST(JSAppTest, InitializeAsSnapshot)
        {
            std::string appName = "testJSAppInitializeSnapshot";

            std::filesystem::path testRoot = s_TestDir / "InitializeAsSnapshot";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            AppProviders providers(std::make_shared<TestSnapshotProvider>(),
                                   std::make_shared<V8RuntimeProvider>(),
                                   std::make_shared<V8ContextProvider>(),
                                   std::make_shared<TestSnapshotCreator>());

            JSAppSharedPtr app = std::make_shared<JSApp>();

            app->Initialize(appName, testRoot, providers, true);
            EXPECT_TRUE(app->Initialize(appName, testRoot, providers, false));
            EXPECT_EQ(providers.m_SnapshotCreator, app->GetSnapshotCreator());
            EXPECT_EQ(providers.m_SnapshotProvider, app->GetSnapshotProvider());
            EXPECT_EQ(providers.m_ContextProvider, app->GetContextProvider());
            EXPECT_EQ(providers.m_RuntimeProvider, app->GetRuntimeProvider());
            EXPECT_NE(nullptr, app->GetMainRuntime());
            EXPECT_NE(nullptr, app->GetCodeCache());
            EXPECT_NE(nullptr, app->GetAppRoot());
            EXPECT_EQ(appName, app->GetName());
            EXPECT_TRUE(app->IsInitialized());
            EXPECT_TRUE(app->IsSnapshotApp());

            app->DisposeApp();
            EXPECT_EQ(nullptr, app->GetSnapshotCreator());
            EXPECT_EQ(nullptr, app->GetSnapshotProvider());
            EXPECT_EQ(nullptr, app->GetContextProvider());
            EXPECT_EQ(nullptr, app->GetRuntimeProvider());
            EXPECT_EQ(nullptr, app->GetMainRuntime());
            EXPECT_EQ(nullptr, app->GetCodeCache());
            EXPECT_EQ(nullptr, app->GetAppRoot());
            EXPECT_EQ(appName, app->GetName());
            EXPECT_FALSE(app->IsInitialized());
            EXPECT_FALSE(app->IsSnapshotApp());
        }

        TEST(JSAppTest, GetCreateDisposeRuntimes)
        {
            std::filesystem::path testRoot = s_TestDir / "GetCreateRuntimes";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::string appName = "testJSAppGetCreateRuntimes";
            AppProviders providers(std::make_shared<TestSnapshotProvider>(),
                                   std::make_shared<V8RuntimeProvider>(),
                                   std::make_shared<V8ContextProvider>(),
                                   std::make_shared<TestSnapshotCreator>());

            JSAppSharedPtr app = std::make_shared<JSApp>();
            app->Initialize(appName, testRoot, providers, false);

            std::string runtimeName1 = "AppRuntime1";
            std::string runtimeName2 = "AppRUNTIME2";

            EXPECT_EQ(nullptr, app->GetRuntimeByName(runtimeName1));

            JSRuntimeSharedPtr runtime = app->CreateJSRuntimeFromIndex(runtimeName1, 0, JSRuntimeSnapshotAttributes::NotSnapshottable, IdleTaskSupport::kEnabled);
            EXPECT_NE(nullptr, runtime);
            EXPECT_EQ(runtime, app->GetRuntimeByName(runtimeName1));
            EXPECT_EQ(nullptr, app->GetRuntimeByName(runtimeName2));

            app->DisposeRuntime(runtimeName1);
            EXPECT_EQ(nullptr, app->GetRuntimeByName(runtimeName1));

            runtime = app->CreateJSRuntimeFromIndex(runtimeName1, 0, JSRuntimeSnapshotAttributes::SnapshotOnly, IdleTaskSupport::kEnabled);
            runtime = app->CreateJSRuntimeFromIndex(runtimeName2, 0, JSRuntimeSnapshotAttributes::SnapshotOnly, IdleTaskSupport::kEnabled);

            app->DisposeApp();
        }
    }
}