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

#include "Assets/BinaryAsset.h"
#include "Assets/TextAsset.h"
#include "Utils/Paths.h"

#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        TEST(JSAppTest, ConstrcutorInitializeDispose)
        {
            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            std::string appName = "testJSAppConstructor";
            JSAppSharedPtr app = std::make_shared<JSApp>(appName, snapProvider);
            EXPECT_EQ(appName, app->GetName());
            EXPECT_EQ(nullptr, app->GetCodeCache());
            EXPECT_EQ(nullptr, app->GetAppRoots());
            EXPECT_EQ(nullptr, app->GetJSRuntime());
            EXPECT_FALSE(app->IsSnapshotCreator());
            EXPECT_FALSE(app->IsInitialized());

            app->InitializeRuntime(s_TestDir, "");
            EXPECT_NE(nullptr, app->GetCodeCache());
            EXPECT_NE(nullptr, app->GetAppRoots());
            EXPECT_NE(nullptr, app->GetJSRuntime());
            EXPECT_EQ(appName + "-runtime", app->GetJSRuntime()->GetName());
            EXPECT_TRUE(app->IsInitialized());

            app->DisposeApp();
            EXPECT_EQ(nullptr, app->GetCodeCache());
            EXPECT_EQ(nullptr, app->GetAppRoots());
            EXPECT_EQ(nullptr, app->GetJSRuntime());
            EXPECT_FALSE(app->IsInitialized());
        }

        TEST(JSAppTest, InitializeAsSnapshot)
        {
            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            std::string appName = "testJSAppInitializeSnapshot";

            JSAppSharedPtr app = std::make_shared<JSApp>(appName, snapProvider);

            app->InitializeRuntime(s_TestDir, "", true);
            EXPECT_NE(nullptr, app->GetCodeCache());
            EXPECT_NE(nullptr, app->GetAppRoots());
            EXPECT_NE(nullptr, app->GetJSRuntime());
            EXPECT_EQ(appName + "-runtime", app->GetJSRuntime()->GetName());
            EXPECT_TRUE(app->IsInitialized());

            app->DisposeApp();
            EXPECT_EQ(nullptr, app->GetCodeCache());
            EXPECT_EQ(nullptr, app->GetAppRoots());
            EXPECT_EQ(nullptr, app->GetJSRuntime());
            EXPECT_FALSE(app->IsInitialized());
        }

        TEST(JSAppTest, GetCreateContext)
        {
            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            std::string appName = "testJSAppGetCreateContext";

            JSAppSharedPtr app = std::make_shared<JSApp>(appName, snapProvider);
            app->InitializeRuntime(s_TestDir, "");
            ASSERT_NE(nullptr, app->GetJSRuntime());

            std::string contextName1 = "AppJSContext1";
            std::string contextName2 = "AppJSContext2";

            EXPECT_EQ(nullptr, app->GetJSContextByName(contextName1));

            JSContextSharedPtr context = app->CreateJSContext(contextName1);
            EXPECT_NE(nullptr, context);
            EXPECT_EQ(context, app->GetJSContextByName(contextName1));
            EXPECT_EQ(nullptr, app->GetJSContextByName(contextName2));
            app->DisposeApp();
        }

        TEST(JSAppTest, EntryPointScript)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            std::string appName = "testJSAppEntryPointScript";
            std::filesystem::path testRoot = s_TestDir /"EntryPointScript";

            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            JSAppSharedPtr app = std::make_shared<JSApp>(appName, snapProvider);
            app->InitializeRuntime(testRoot, "");

            EXPECT_EQ("", app->GetEntryPointScript().generic_string());

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            // create the required directroies for the app root
            std::filesystem::path testFile("../../test.js");

            // path escapes the app root
            EXPECT_FALSE(app->SetEntryPointScript(testFile));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Passed entry point script may have escaped the app root. File:{}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // path doesn't exist
            testFile = testRoot / "js/test.js";
            EXPECT_FALSE(app->SetEntryPointScript(testFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Passed entry point script doesn't exist {}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // should succeed
            {
                Assets::TextAsset file(testFile);
                file.SetContent("test");
                EXPECT_TRUE(file.WriteAsset());
            }
            EXPECT_TRUE(app->SetEntryPointScript(testFile));
            std::filesystem::remove(testFile);

            app->DisposeApp();
        }

        TEST(JSAppTest, V8BaseSnapshotProviderLoadSnapshotData)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);

            std::shared_ptr<V8BaseSnapshotProvider> testingSnapProvider = std::make_shared<V8BaseSnapshotProvider>();
            std::string appName = "testJSAppV8BaseSnapshotProviderLoadSnapshotData";
            
            std::filesystem::path testRoot = s_TestDir /"SnapshotProviderTest";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            //need the test snapshot provider so we can init the app
            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            JSAppSharedPtr app = std::make_shared<JSApp>(appName, snapProvider);
            app->InitializeRuntime(testRoot, "");

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            std::filesystem::path testFile("../../test.dat");

            EXPECT_EQ(nullptr, testingSnapProvider->GetSnapshotData()->data);
            EXPECT_EQ(0, testingSnapProvider->GetSnapshotData()->raw_size);

            // path escapes the app root
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(testFile, app));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, Utils::format("Passed snapshot data may have escaped the app root. File:{}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // path doesn't exist
            testFile = testRoot / "resources/test.js";
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(testFile, app));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Passed snapshot data doesn't exist {}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            {
                Assets::BinaryAsset dataFile(testFile);
                Assets::BinaryByteVector data{1, 2, 3, 4};
                dataFile.SetContent(data);
                EXPECT_TRUE(dataFile.WriteAsset());
            }
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(testFile, app));
            EXPECT_EQ(4, testingSnapProvider->GetSnapshotData()->raw_size);

            std::filesystem::remove(testFile);
            app->DisposeApp();
        }

    }
}