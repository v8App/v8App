// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "TestLogSink.h"
#include "TestFiles.h"

#include "Assets/BinaryAsset.h"
#include "Assets/TextAsset.h"
#include "Utils/Paths.h"

#include "TestSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        TEST(V8SnapshotProvider, LoadSnapshotData)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);
            logSink->FlushMessages();

            std::filesystem::path testRoot = s_TestDir / "SnapshotProviderTest";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::shared_ptr<V8SnapshotProvider> testingSnapProvider = std::make_shared<V8SnapshotProvider>();
            std::string appName = "testJSAppV8BaseSnapshotProviderLoadSnapshotData";

            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            JSAppSharedPtr app = std::make_shared<JSApp>(appName, snapProvider);
            app->Initialize(testRoot);

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            std::filesystem::path testFile("../../test.dat");

            EXPECT_EQ(nullptr, testingSnapProvider->GetSnapshotData()->data);
            EXPECT_EQ(0, testingSnapProvider->GetSnapshotData()->raw_size);
            EXPECT_EQ("", testingSnapProvider->GetSnapshotPath().generic_string());

            // no path passed on construction or in function call
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(app));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "A path needs to be specified at construction or passed to LoadSnapshotData"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // path escapes the app root
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(app, testFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Specified snapshot path may have escaped the app root. File:{}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // path doesn't exist
            testFile = testRoot / "resources/test.js";
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(app, testFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // loads the snapshot file
            {
                Assets::BinaryAsset dataFile(testFile);
                Assets::BinaryByteVector data{1, 2, 3, 4};
                dataFile.SetContent(data);
                EXPECT_TRUE(dataFile.WriteAsset());
            }
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(app, testFile));
            EXPECT_EQ(4, testingSnapProvider->GetSnapshotData()->raw_size);
            EXPECT_TRUE(testingSnapProvider->SnapshotLoaded());

            // return true since data is already loaded
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(app));

            // loads via path in constructor
            testingSnapProvider = std::make_shared<V8SnapshotProvider>(testFile);
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(app, testFile));
            EXPECT_EQ(4, testingSnapProvider->GetSnapshotData()->raw_size);

            std::filesystem::remove(testFile);
            app->DisposeApp();
        }
    }
}