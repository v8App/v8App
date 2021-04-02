// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "Logging/LogMacros.h"

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
                TestSink(std::string inName)
                {
                    m_Name = inName;
                }

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

        TEST(LogTest, LogLevelToString)
        {
            EXPECT_EQ("Fatal", LogLevelToString(LogLevel::Fatal));
            EXPECT_EQ("Off", LogLevelToString(LogLevel::Off));
            EXPECT_EQ("Error", LogLevelToString(LogLevel::Error));
            EXPECT_EQ("General", LogLevelToString(LogLevel::General));
            EXPECT_EQ("Warn", LogLevelToString(LogLevel::Warn));
            EXPECT_EQ("Debug", LogLevelToString(LogLevel::Debug));
            EXPECT_EQ("Trace", LogLevelToString(LogLevel::Trace));
        }

        TEST(LogTest, GetSetLogLevel)
        {
            EXPECT_EQ(LogLevel::Error, Log::GetLogLevel());
            Log::SetLogLevel(LogLevel::Off);
            EXPECT_EQ(LogLevel::Off, Log::GetLogLevel());
            Log::SetLogLevel(LogLevel::Fatal);
            EXPECT_EQ(LogLevel::Off, Log::GetLogLevel());
        }

        TEST(LogTest, GetSetAppName)
        {
            EXPECT_EQ("v8App", Log::GetAppName());
            std::string testName("TestAppName");
            Log::SetAppName(testName);
            EXPECT_EQ(testName, Log::GetAppName());
        }

        TEST(LogTest, UseUTC)
        {
            EXPECT_TRUE(Log::IsUsingUTC());
            Log::UseUTC(false);
            EXPECT_FALSE(Log::IsUsingUTC());
            Log::UseUTC(true);
            EXPECT_TRUE(Log::IsUsingUTC());
        }

        TEST(LogTest, AddRemoveLogSink)
        {
            //NOTE: the unique_ptr owns this and will delete it when it's removed
            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            std::unique_ptr<ILogSink> sink(sinkObj);
            std::unique_ptr<ILogSink> sink2(new LogTest::TestSink("TestSink"));

            //need to change the log level so we can see the warn
            Log::SetLogLevel(LogLevel::Warn);

            ASSERT_TRUE(Log::AddLogSink(sink));
            ASSERT_FALSE(Log::AddLogSink(sink2));
            EXPECT_TRUE(sinkObj->WantsLogMessage(LogLevel::Warn));
            ASSERT_FALSE(sinkObj->m_Message.empty());

            //there are other keys in the message but we are only concerned with the msg for this test.
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::Msg) != sinkObj->m_Message.end());
            ASSERT_EQ("Sink with name:" + sinkObj->GetName() + " already exists", sinkObj->m_Message.at(MsgKey::Msg));

            //ok time to remove it.
            Log::RemoveLogSink(sinkObj->GetName());
            //at this point the sinkObj isn't valid;
            sinkObj = nullptr;
            ASSERT_TRUE(LogTest::LogDouble::IsSinksEmpty());
        }

        TEST(LogTest, GenerateISO8601Time)
        {
            std::string time = Log::GenerateISO8601Time(true);
            //utc version
            //EXPECT_THAT(Log::GenerateISO8601Time(true), ::testing::MatchesRegex("\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\dZ"));
            //non utc version
            //EXPECT_THAT(Log::GenerateISO8601Time(false), ::testing::MatchesRegex("\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d"));
        }

        TEST(LogTest, testInternalLogExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");
            Log::SetLogLevel(LogLevel::General);

            ::testing::internal::CaptureStderr();
            LogTest::LogDouble::TestInternalLog(message, LogLevel::General, "File", "Function", 10);
            std::string output = ::testing::internal::GetCapturedStderr();

            EXPECT_THAT(output, ::testing::HasSubstr(Log::GetAppName() + " Log {"));
            EXPECT_THAT(output, ::testing::HasSubstr("Message:Test"));
            EXPECT_THAT(output, ::testing::HasSubstr("File:File"));
            EXPECT_THAT(output, ::testing::HasSubstr("Function:Function"));
            EXPECT_THAT(output, ::testing::HasSubstr("Line:10"));
            EXPECT_THAT(output, ::testing::HasSubstr("}"));

            //test with the test sink
            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));
            LogTest::LogDouble::TestInternalLog(message, LogLevel::General, "File", "Function", 10);

            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::AppName) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::TimeStamp) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::Msg) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::File) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::Function) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::Line) != sinkObj->m_Message.end());

            ASSERT_EQ(Log::GetAppName(), sinkObj->m_Message.at(MsgKey::AppName));
            ASSERT_EQ("General", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));
        }

        TEST(LogTest, testInternalLog)
        {
            LogTest::LogDouble::ResetLog();

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");
            Log::SetLogLevel(LogLevel::General);

            //first test with no sinks
            ::testing::internal::CaptureStderr();
            LogTest::LogDouble::TestInternalLog(message, LogLevel::General);
            std::string output = ::testing::internal::GetCapturedStderr();

            EXPECT_THAT(output, ::testing::HasSubstr(Log::GetAppName() + " Log {"));
            EXPECT_THAT(output, ::testing::HasSubstr("Message:Test"));
            EXPECT_THAT(output, ::testing::HasSubstr("}"));

            //test with the test sink
            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));
            LogTest::LogDouble::TestInternalLog(message, LogLevel::General);

            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::AppName) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::TimeStamp) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::Msg) != sinkObj->m_Message.end());

            ASSERT_EQ(Log::GetAppName(), sinkObj->m_Message.at(MsgKey::AppName));
            ASSERT_EQ("General", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));

            //test not logged to sink cause it doesn't want level
            sinkObj->m_Message.clear();
            LogTest::LogDouble::TestInternalLog(message, LogLevel::Error);
            ASSERT_TRUE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testError)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Error(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Error);

            Log::Error(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Error", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::General);
            sinkObj->m_Message.clear();
            Log::Error(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());

        }

        TEST(LogTest, testGeneral)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Error);

            Log::General(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::General(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::General);

            Log::General(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("General", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::Warn);
            sinkObj->m_Message.clear();
            Log::General(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testWarn)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::General);

            Log::Warn(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Warn(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Warn(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Warn", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::Debug);
            sinkObj->m_Message.clear();
            Log::Warn(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testDebug)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Debug(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Debug(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Debug(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Debug", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::Trace);
            sinkObj->m_Message.clear();
            Log::Debug(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testTrace)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Trace(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

           //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Trace(message);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Trace);

            Log::Trace(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Trace", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
       }

        TEST(LogTest, testFatal)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //test sends message even though off and sink doesn't want it.
            Log::SetLogLevel(LogLevel::Off);

            Log::Fatal(message);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Fatal", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
        }

        TEST(LogTest, testErrorExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

 
            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Error(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());


            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Error);

            Log::Error(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Error", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));
 
            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::General);
            sinkObj->m_Message.clear();
            Log::Error(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testGeneralExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Error);

            Log::General(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::General(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::General);

            Log::General(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("General", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::Warn);
            sinkObj->m_Message.clear();
            Log::General(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testWarnExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::General);

            Log::Warn(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Warn(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Warn(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Warn", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::Debug);
            sinkObj->m_Message.clear();
            Log::Warn(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testDebugExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Debug(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Debug(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Debug(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Debug", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));

            //tets that the message logged on higher level
            Log::SetLogLevel(LogLevel::Trace);
            sinkObj->m_Message.clear();
            Log::Debug(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
        }

        TEST(LogTest, testTraceExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Trace(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //tets no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Trace(message, "File", "Function", 10);
            EXPECT_TRUE(sinkObj->m_Message.empty());

            //test message gets logged on level
            Log::SetLogLevel(LogLevel::Trace);

            Log::Trace(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Trace", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));
     }

        TEST(LogTest, testFatalExtended)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            //test message gets ent even though off and sink doesn't want it
            Log::SetLogLevel(LogLevel::Trace);

            Log::Fatal(message, "File", "Function", 10);
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Fatal", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
            ASSERT_EQ("File", sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ("Function", sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ("10", sinkObj->m_Message.at(MsgKey::Line));
        }

        TEST(LogTest, testLogMacroError)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            sinkObj->m_Message.clear();

            LOG_ERROR(message);
            //so the checks aren't as brittle
            size_t line = __LINE__-2;
            std::string sLine = std::to_string(line);
            const char* file = __FILE__;
            const char* func = __func__;
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Error", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
#if V8APP_DEBUG
            ASSERT_EQ(file, sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ(func, sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ(sLine, sinkObj->m_Message.at(MsgKey::Line));
#endif
        }
 
        TEST(LogTest, testLogMacroGeneral)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_GENERAL(message);
            //so the checks aren't as brittle
            size_t line = __LINE__-2;
            std::string sLine = std::to_string(line);
            const char* file = __FILE__;
            const char* func = __func__;
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("General", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
#if V8APP_DEBUG
            ASSERT_EQ(file, sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ(func, sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ(sLine, sinkObj->m_Message.at(MsgKey::Line));
#endif
        }

       TEST(LogTest, testLogMacroWarn)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

 
            sinkObj->m_Message.clear();

            LOG_WARN(message);
             //so the checks aren't as brittle
            size_t line = __LINE__-2;
            std::string sLine = std::to_string(line);
            const char* file = __FILE__;
            const char* func = __func__;
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Warn", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
#if V8APP_DEBUG
            ASSERT_EQ(file, sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ(func, sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ(sLine, sinkObj->m_Message.at(MsgKey::Line));
#endif
        }

        TEST(LogTest, testLogMacroDebug)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_DEBUG(message);
            //so the checks aren't as brittle
            size_t line = __LINE__-2;
            std::string sLine = std::to_string(line);
            const char* file = __FILE__;
            const char* func = __func__;
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Debug", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
#if V8APP_DEBUG
            ASSERT_EQ(file, sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ(func, sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ(sLine, sinkObj->m_Message.at(MsgKey::Line));
#endif
        }

        TEST(LogTest, testLogMacroTrace)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_TRACE(message);
            //so the checks aren't as brittle
            size_t line = __LINE__-2;
            std::string sLine = std::to_string(line);
            const char* file = __FILE__;
            const char* func = __func__;
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Trace", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
#if V8APP_DEBUG
            ASSERT_EQ(file, sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ(func, sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ(sLine, sinkObj->m_Message.at(MsgKey::Line));
#endif
        }

        TEST(LogTest, testLogMacroFatal)
        {
            LogTest::LogDouble::ResetLog();

            LogTest::TestSink *sinkObj = new LogTest::TestSink("TestSink");
            sinkObj->m_WantsLevels.push_back(LogLevel::Error);
            sinkObj->m_WantsLevels.push_back(LogLevel::General);
            sinkObj->m_WantsLevels.push_back(LogLevel::Warn);
            sinkObj->m_WantsLevels.push_back(LogLevel::Debug);
            sinkObj->m_WantsLevels.push_back(LogLevel::Trace);
            std::unique_ptr<ILogSink> sink(sinkObj);
            ASSERT_TRUE(Log::AddLogSink(sink));

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_FATAL(message);
            //so the checks aren't as brittle
            size_t line = __LINE__-2;
            std::string sLine = std::to_string(line);
            const char* file = __FILE__;
            const char* func = __func__;
            ASSERT_FALSE(sinkObj->m_Message.empty());
            ASSERT_TRUE(sinkObj->m_Message.find(MsgKey::LogLevel) != sinkObj->m_Message.end());
            ASSERT_EQ("Fatal", sinkObj->m_Message.at(MsgKey::LogLevel));
            ASSERT_EQ("Test", sinkObj->m_Message.at(MsgKey::Msg));
#if V8APP_DEBUG
            ASSERT_EQ(file, sinkObj->m_Message.at(MsgKey::File));
            ASSERT_EQ(func, sinkObj->m_Message.at(MsgKey::Function));
            ASSERT_EQ(sLine, sinkObj->m_Message.at(MsgKey::Line));
#endif

        }
   } // namespace Log
} // namespace v8App