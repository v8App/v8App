// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Logging/Log.h"
#include "Logging/ILogSink.h"
#include "Logging/LogMacros.h"
#include "TestLogSink.h"

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

            TestUtils::TestLogSink *SetupGlobalSink(TestUtils::WantsLogLevelsVector inLevels)
            {
                TestUtils::TestLogSink *sink = TestUtils::TestLogSink::GetGlobalSink();
                sink->SetWantsLogLevels(inLevels);
                sink->FlushMessages();
                return sink;
            }

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
            // NOTE: the unique_ptr owns this and will delete it when it's removed
            TestUtils::WantsLogLevelsVector warns = {LogLevel::Warn};
            TestUtils::TestLogSink *testSink = new TestUtils::TestLogSink("TestLogSink", warns);
            std::unique_ptr<ILogSink> sinkObj(testSink);

            TestUtils::WantsLogLevelsVector emptyWants;
            std::unique_ptr<ILogSink> sink2(new TestUtils::TestLogSink("TestLogSink", emptyWants));

            // need to change the log level so we can see the warn
            Log::SetLogLevel(LogLevel::Warn);

            ASSERT_TRUE(Log::AddLogSink(sinkObj));
            ASSERT_FALSE(Log::AddLogSink(sink2));
            EXPECT_TRUE(testSink->WantsLogMessage(LogLevel::Warn));

            // there are other keys in the message but we are only concerned with the msg for this test.
            LogMessage expected = {
                {MsgKey::Msg, "Sink with name:" + testSink->GetName() + " already exists"},
                {MsgKey::LogLevel, "Warn"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::AppName, MsgKey::TimeStamp};
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // ok time to remove it.
            Log::RemoveLogSink(testSink->GetName());
            // at this point the sinkObj isn't valid;
            sinkObj = nullptr;
            ASSERT_TRUE(LogTest::LogDouble::IsSinksEmpty());
        }

        TEST(LogTest, GenerateISO8601Time)
        {
            std::string time = Log::GenerateISO8601Time(true);
            // utc version
            // EXPECT_THAT(Log::GenerateISO8601Time(true), ::testing::MatchesRegex("\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\dZ"));
            // non utc version
            // EXPECT_THAT(Log::GenerateISO8601Time(false), ::testing::MatchesRegex("\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d"));
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

            // test with the test sink
            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::General});

            LogTest::LogDouble::TestInternalLog(message, LogLevel::General, "File", "Function", 10);

            LogMessage expected = {
                {MsgKey::AppName, Log::GetAppName()},
                {MsgKey::LogLevel, "General"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp};
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testInternalLog)
        {
            LogTest::LogDouble::ResetLog();

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");
            Log::SetLogLevel(LogLevel::General);

            // first test with no sinks
            ::testing::internal::CaptureStderr();
            LogTest::LogDouble::TestInternalLog(message, LogLevel::General);
            std::string output = ::testing::internal::GetCapturedStderr();

            EXPECT_THAT(output, ::testing::HasSubstr(Log::GetAppName() + " Log {"));
            EXPECT_THAT(output, ::testing::HasSubstr("Message:Test"));
            EXPECT_THAT(output, ::testing::HasSubstr("}"));

            // test with the test sink
            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::General});

            LogTest::LogDouble::TestInternalLog(message, LogLevel::General);

            LogMessage expected = {
                {MsgKey::AppName, Log::GetAppName()},
                {MsgKey::LogLevel, "General"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp};
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test not logged to sink cause it doesn't want level
            testSink->FlushMessages();
            LogTest::LogDouble::TestInternalLog(message, LogLevel::Error);
            ASSERT_TRUE(testSink->NoMessages());
        }

        TEST(LogTest, testError)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Error(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Error);

            Log::Error(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Error"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::General);
            testSink->FlushMessages();
            Log::Error(message);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testGeneral)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Error);

            Log::General(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::General(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::General);

            Log::General(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "General"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::Warn);
            testSink->FlushMessages();
            Log::General(message);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testWarn)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Warn, LogLevel::General});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::General);

            Log::Warn(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Warn(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Warn(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Warn"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::Debug);
            testSink->FlushMessages();
            Log::Warn(message);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testDebug)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Debug, LogLevel::Warn});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Debug(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Debug(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Debug(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Debug"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::Trace);
            testSink->FlushMessages();
            Log::Debug(message);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testTrace)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Trace, LogLevel::Debug});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Trace(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Trace(message);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Trace);

            Log::Trace(message);
            LogMessage expected = {
                {MsgKey::LogLevel, "Trace"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testFatal)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // test sends message even though off and sink doesn't want it.
            Log::SetLogLevel(LogLevel::Off);

            Log::Fatal(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Fatal"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testErrorExtended)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Error(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Error);

            Log::Error(message, "File", "Function", 10);

            LogMessage expected = {
                {MsgKey::LogLevel, "Error"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::General);
            testSink->FlushMessages();
            Log::Error(message, "File", "Function", 10);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testGeneralExtended)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Error);

            Log::General(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::General(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::General);

            Log::General(message, "File", "Function", 10);
            LogMessage expected = {
                {MsgKey::LogLevel, "General"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::Warn);
            testSink->FlushMessages();
            Log::General(message, "File", "Function", 10);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testWarnExtended)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Warn, LogLevel::General});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::General);

            Log::Warn(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Warn(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Warn(message, "File", "Function", 10);

            LogMessage expected = {
                {MsgKey::LogLevel, "Warn"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::Debug);
            testSink->FlushMessages();
            Log::Warn(message, "File", "Function", 10);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testDebugExtended)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Debug, LogLevel::Warn});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Warn);

            Log::Debug(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Debug(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Debug(message, "File", "Function", 10);

            LogMessage expected = {
                {MsgKey::LogLevel, "Debug"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));

            // test that the message logged on higher level
            Log::SetLogLevel(LogLevel::Trace);
            testSink->FlushMessages();
            Log::Debug(message, "File", "Function", 10);
            ASSERT_FALSE(testSink->NoMessages());
        }

        TEST(LogTest, testTraceExtended)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Trace, LogLevel::Debug});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // first test the message doesn't get logged on lower level
            Log::SetLogLevel(LogLevel::Debug);

            Log::Trace(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test no message when level is set to off.
            Log::SetLogLevel(LogLevel::Off);

            Log::Trace(message, "File", "Function", 10);
            EXPECT_TRUE(testSink->NoMessages());

            // test message gets logged on level
            Log::SetLogLevel(LogLevel::Trace);

            Log::Trace(message, "File", "Function", 10);

            LogMessage expected = {
                {MsgKey::LogLevel, "Trace"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testFatalExtended)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            // test message gets ent even though off and sink doesn't want it
            Log::SetLogLevel(LogLevel::Trace);

            Log::Fatal(message, "File", "Function", 10);

            LogMessage expected = {
                {MsgKey::LogLevel, "Fatal"},
                {MsgKey::Msg, "Test"},
                {MsgKey::File, "File"},
                {MsgKey::Function, "Function"},
                {MsgKey::Line, "10"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};

            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testLogMacroError)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General, LogLevel::Warn, LogLevel::Debug, LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            testSink->FlushMessages();

            LOG_ERROR(message);
            ASSERT_FALSE(testSink->NoMessages());

            LogMessage expected = {
                {MsgKey::LogLevel, "Error"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};
#ifdef V8APP_DEBUG
            // so the checks aren't as brittle
            size_t line = 675;
            std::string sLine = std::to_string(line);
            const char *file = __FILE__;
            const char *func = __func__;

            expected.emplace(MsgKey::File, file);
            expected.emplace(MsgKey::Function, func);
            expected.emplace(MsgKey::Line, sLine);
#endif
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testLogMacroGeneral)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General, LogLevel::Warn, LogLevel::Debug, LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_GENERAL(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "General"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};
#ifdef V8APP_DEBUG
            // so the checks aren't as brittle
            size_t line = 709;
            std::string sLine = std::to_string(line);
            const char *file = __FILE__;
            const char *func = __func__;

            expected.emplace(MsgKey::File, file);
            expected.emplace(MsgKey::Function, func);
            expected.emplace(MsgKey::Line, sLine);
#endif
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testLogMacroWarn)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General, LogLevel::Warn, LogLevel::Debug, LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            testSink->FlushMessages();

            LOG_WARN(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Warn"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};
#ifdef V8APP_DEBUG
            // so the checks aren't as brittle
            size_t line = 744;
            std::string sLine = std::to_string(line);
            const char *file = __FILE__;
            const char *func = __func__;

            expected.emplace(MsgKey::File, file);
            expected.emplace(MsgKey::Function, func);
            expected.emplace(MsgKey::Line, sLine);
#endif
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testLogMacroDebug)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General, LogLevel::Warn, LogLevel::Debug, LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_DEBUG(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Debug"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};
#ifdef V8APP_DEBUG
            // so the checks aren't as brittle
            size_t line = 777;
            std::string sLine = std::to_string(line);
            const char *file = __FILE__;
            const char *func = __func__;

            expected.emplace(MsgKey::File, file);
            expected.emplace(MsgKey::Function, func);
            expected.emplace(MsgKey::Line, sLine);
#endif
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testLogMacroTrace)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General, LogLevel::Warn, LogLevel::Debug, LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_TRACE(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Trace"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};
#ifdef V8APP_DEBUG
            // so the checks aren't as brittle
            size_t line = 810;
            std::string sLine = std::to_string(line);
            const char *file = __FILE__;
            const char *func = __func__;

            expected.emplace(MsgKey::File, file);
            expected.emplace(MsgKey::Function, func);
            expected.emplace(MsgKey::Line, sLine);
#endif
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }

        TEST(LogTest, testLogMacroFatal)
        {
            LogTest::LogDouble::ResetLog();

            TestUtils::TestLogSink *testSink = LogTest::SetupGlobalSink({LogLevel::Error, LogLevel::General, LogLevel::Warn, LogLevel::Debug, LogLevel::Trace});

            LogMessage message;
            message.emplace(MsgKey::Msg, "Test");

            Log::SetLogLevel(LogLevel::Trace);

            LOG_FATAL(message);

            LogMessage expected = {
                {MsgKey::LogLevel, "Fatal"},
                {MsgKey::Msg, "Test"},
            };

            TestUtils::IgnoreMsgKeys ignore = {MsgKey::TimeStamp, MsgKey::AppName};
#ifdef V8APP_DEBUG
            // so the checks aren't as brittle
            size_t line = 843;
            std::string sLine = std::to_string(line);
            const char *file = __FILE__;
            const char *func = __func__;

            expected.emplace(MsgKey::File, file);
            expected.emplace(MsgKey::Function, func);
            expected.emplace(MsgKey::Line, sLine);
#endif
            ASSERT_TRUE(testSink->ValidateMessage(expected, ignore));
        }
    } // namespace Log
} // namespace v8App