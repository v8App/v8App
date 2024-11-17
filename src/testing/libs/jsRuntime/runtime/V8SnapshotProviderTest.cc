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
#include "Utils/Format.h"

#include "JSApp.h"
#include "V8ContextProvider.h"
#include "V8RuntimeProvider.h"
#include "V8SnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        TEST(V8SnapshotProviderTest, LoadSnapshotData)
        {
            TestUtils::TestLogSink *logSink = TestUtils::TestLogSink::GetGlobalSink();
            Log::Log::SetLogLevel(Log::LogLevel::Error);
            logSink->FlushMessages();

            std::filesystem::path testRoot = s_TestDir / "V8SnapshotProviderTest";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::shared_ptr<V8SnapshotProvider> testingSnapProvider = std::make_shared<V8SnapshotProvider>();
            std::string appName = "testJSAppV8BaseSnapshotProviderLoadSnapshotData";

            AppProviders providers(std::make_shared<TestSnapshotProvider>(),
                                   std::make_shared<V8RuntimeProvider>(),
                                   std::make_shared<V8ContextProvider>());
            JSAppSharedPtr app = std::make_shared<JSApp>();
            app->Initialize(appName, testRoot, providers);

            TestUtils::IgnoreMsgKeys ignoreKeys = {
                Log::MsgKey::AppName,
                Log::MsgKey::TimeStamp,
                Log::MsgKey::File,
                Log::MsgKey::Function,
                Log::MsgKey::Line};

            std::filesystem::path testFile("../../test.dat");
            std::filesystem::path testFile2(testRoot / "resources/test2.dat");

            EXPECT_EQ(nullptr, testingSnapProvider->GetSnapshotData()->data);
            EXPECT_EQ(0, testingSnapProvider->GetSnapshotData()->raw_size);
            EXPECT_EQ("", testingSnapProvider->GetSnapshotPath().generic_string());

            // no path passed on construction or in function call
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(""));
            Log::LogMessage expected = {
                {Log::MsgKey::Msg, "A path needs to be passed to LoadSnapshotData"},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // path escapes the app root
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(testFile));
            expected = {
                {Log::MsgKey::Msg, Utils::format("Specified snapshot path may have escaped the app root. File:{}", testFile)},
                {Log::MsgKey::LogLevel, "Error"}};
            EXPECT_TRUE(logSink->ValidateMessage(expected, ignoreKeys));

            // path doesn't exist
            testFile = testRoot / "resources/test.dat";
            EXPECT_FALSE(testingSnapProvider->LoadSnapshotData(testFile));
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

                Assets::BinaryAsset dataFile2(testFile2);
                Assets::BinaryByteVector data2{5, 6};
                dataFile2.SetContent(data2);
                EXPECT_TRUE(dataFile2.WriteAsset());
            }
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(testFile));
            EXPECT_EQ(4, testingSnapProvider->GetSnapshotData()->raw_size);
            EXPECT_TRUE(testingSnapProvider->SnapshotLoaded());

            // return true since data is already loaded
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(testFile));

            // loads new file over old
            testingSnapProvider = std::make_shared<V8SnapshotProvider>();
            EXPECT_TRUE(testingSnapProvider->LoadSnapshotData(testFile2));
            EXPECT_EQ(2, testingSnapProvider->GetSnapshotData()->raw_size);

            std::filesystem::remove(testFile);
            std::filesystem::remove(testFile2);
            app->DisposeApp();
        }
    }
}