// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"

#include "Logging/LogJSONFile.h"
#include "Assets/TextAsset.h"
#include "Utils/Paths.h"

namespace v8App
{
    namespace Log
    {
        TEST(LogJSONFileTest, Constructor)
        {
            LogJSONFile log("Test", std::filesystem::path("Test"));

            EXPECT_EQ("Test", log.GetFilePath().string());
            EXPECT_EQ("Test", log.GetName());
        }

        TEST(LogJSONFileTest, WantsLogMessage)
        {
            LogJSONFile log("Test", std::filesystem::path("Test"));

            EXPECT_TRUE(log.WantsLogMessage(LogLevel::Debug));
            EXPECT_TRUE(log.WantsLogMessage(LogLevel::Error));
            EXPECT_TRUE(log.WantsLogMessage(LogLevel::Fatal));
            EXPECT_TRUE(log.WantsLogMessage(LogLevel::General));
            EXPECT_TRUE(log.WantsLogMessage(LogLevel::Trace));
            EXPECT_TRUE(log.WantsLogMessage(LogLevel::Warn));
            EXPECT_FALSE(log.WantsLogMessage(LogLevel::Off));
        }

        TEST(LogJSONFileTest, SinkMessage)
        {
            std::filesystem::path logFile = s_TestDir / "log/JSONTest.log";
            std::filesystem::create_directory(s_TestDir /"log");

            std::unique_ptr<ILogSink> log = std::make_unique<LogJSONFile>("Test", logFile);

            // test new file
            LogMessage testMessage = {
                {"Test", "Test"},
                {"App", "Test"},
                };

            log->SinkMessage(testMessage);
            log.reset();

            Assets::TextAsset logOutput(logFile);
            EXPECT_TRUE(logOutput.ReadAsset());
            EXPECT_EQ(logOutput.GetContent(), "{\"App\":\"Test\",\"Test\":\"Test\"}\n");

            // test appending file
            log = std::make_unique<LogJSONFile>("Test", logFile);
            log->SinkMessage(testMessage);
            log.reset();

            EXPECT_TRUE(logOutput.ReadAsset());
            EXPECT_EQ(logOutput.GetContent(), "{\"App\":\"Test\",\"Test\":\"Test\"}\n{\"App\":\"Test\",\"Test\":\"Test\"}\n");

            // fail to open file
            logFile = s_TestDir / "log/JSONTestDir.log";
            std::filesystem::create_directory(logFile);
            testing::internal::CaptureStderr();
            log = std::make_unique<LogJSONFile>("Test", logFile);
            log->SinkMessage(testMessage);
            log.reset();

            std::string message = "Failed to open JSON log file: \"" + logFile.string() +"\""
            + "\nLog: {\"App\":\"Test\",\"Test\":\"Test\"}\n";
            EXPECT_EQ(testing::internal::GetCapturedStderr(), message);
        }

    }
}