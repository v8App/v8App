// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "Logging/LogMacros.h"
#include "Assets/TextAsset.h"
#include "Logging/LogJSONFile.h"
#include "test_main.h"

namespace v8App
{
    namespace Log
    {
        namespace LogTest
        {
            class LogDouble : public Log
            {
            public:
                static bool IsSinksEmpty() { return m_LogSinks.empty(); }
                static void TestInternalLog(LogMessage &inMessage, LogLevel inLevel, std::string File, std::string Function, int Line)
                {
                    InternalLog(inMessage, inLevel, File, Function, Line);
                }
                static void TestInternalLog(LogMessage &inMessage, LogLevel inLevel)
                {
                    InternalLog(inMessage, inLevel);
                }
                static void ResetLog()
                {
                    m_LogSinks.clear();
                    m_AppName = "v8App";
                    m_LogLevel = LogLevel::Error;
                    m_UseUTC = true;
                }
            };

            class TestSink : public ILogSink
            {
            public:
                TestSink(std::string inName) : ILogSink(inName) {}

                virtual ~TestSink() {}

                virtual void SinkMessage(LogMessage &inMessage)
                {
                    m_Message = inMessage;
                }

                virtual bool WantsLogMessage(LogLevel inLevel)
                {
                    return (std::find(m_WantsLevels.begin(), m_WantsLevels.end(), inLevel) != m_WantsLevels.end());
                }

                LogMessage m_Message;
                std::vector<LogLevel> m_WantsLevels;
            };

        } // namespace LogTest

        TEST(LogDeathTest, CheckFullMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_FULL(1 == 0, "File", "Function", 10); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_FULL(1 == 1, "File", "Function", 10);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckUnimplemented)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ UNIMPLEMENTED(); }, "v8App Log \\{");
        }

        TEST(LogDeathTest, CheckUnreachable)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ UNREACHABLE(); }, "v8App Log \\{");
        }

        TEST(LogDeathTest, CheckMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK(1 == 0); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK(1 == 1);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckEqualMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_EQ(1, 0); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_EQ(1, 1);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckNotEqualMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_NE(1, 1); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_NE(1, 0);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckGreaterThanMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_GT(1, 5); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_GT(1, 0);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckGreaterThanEqualMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_GE(1, 5); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_GE(1, 1);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");

            EXPECT_EXIT({
                CHECK_GE(1, 0);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckLessThanMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_LT(1, 0); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_LT(0, 1);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckLessThanEqualMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_LE(1, 0); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_LE(1, 1);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");

            EXPECT_EXIT({
                CHECK_LE(0, 1);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckNullMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({
                int n = 5;
                CHECK_NULL(&n); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_NULL(nullptr);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckNotNullMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_NOT_NULL(nullptr); }, "v8App Log \\{");

            EXPECT_EXIT({
                int n = 5;
                CHECK_NOT_NULL(&n);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckTrueMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_TRUE(false); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_TRUE(true);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckFalseMacro)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            // The output for this is really a multiline message however
            //  the assert only checks the first line against what's passed
            // which is why it's just the first line of the log message.
            ASSERT_DEATH({ CHECK_FALSE(true); }, "v8App Log \\{");

            EXPECT_EXIT({
                CHECK_FALSE(false);
                std::exit(0); }, ::testing::ExitedWithCode(0), "");
        }

        TEST(LogDeathTest, CheckLogShutdown)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            LogTest::LogDouble::ResetLog();

            std::filesystem::path logFile = s_TestDir / "log/LogShutdownAbort.log";
            std::filesystem::create_directory(s_TestDir / "log");

            ASSERT_DEATH({
             std::filesystem::path logFile = s_TestDir / "log/LogShutdownAbort.log";
               std::unique_ptr<ILogSink> log = std::make_unique<LogJSONFile>("Test", logFile);
               Log::Log::AddLogSink(log);
            UNREACHABLE(); }, "");

            Assets::TextAsset logOutput(logFile);
            EXPECT_TRUE(logOutput.ReadAsset());
            std::string content = logOutput.GetContent();
            EXPECT_TRUE(content.find("Hit a supposedly unreacable code point") != content.npos);
        }
    } // namespace Log
} // namespace v8App