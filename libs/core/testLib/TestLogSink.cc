// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <queue>
#include <iostream>
#include <algorithm>

#include "Logging/Log.h"
#include "TestLogSink.h"

namespace v8App
{
    namespace TestUtils
    {
        const std::string TestLogSink::GlobalTestSinkName("GlobalTestLogSink");

        TestLogSink *TestLogSink::GetGlobalSink()
        {
            ILogSink* sink = Log::Log::GetLogSink(GlobalTestSinkName);
            TestLogSink* testSink;

            //if we get backa sink then we need to make sure it's a TestLogSink
            if(sink != nullptr)
            {
                testSink = dynamic_cast<TestLogSink*>(sink);
                if(testSink != nullptr)
                {
                    return testSink;
                }

                //It's not so remove it and we'll readd it.
                Log::Log::RemoveLogSink(GlobalTestSinkName);
            } 

            //By default we want everything but it can be changed
            WantsLogLevelsVector wants = {
                Log::LogLevel::Error,
                Log::LogLevel::General,
                Log::LogLevel::Warn,
                Log::LogLevel::Debug,
                Log::LogLevel::Trace,
            };

            testSink = new TestLogSink(GlobalTestSinkName, wants);
            std::unique_ptr<ILogSink> uTestSink(testSink);
            Log::Log::AddLogSink(uTestSink);

            return testSink;
        }

        TestLogSink::TestLogSink(const std::string &inName, const WantsLogLevelsVector &inWantLevels)
            : ILogSink(inName), m_WantsLevels(inWantLevels)
        {
        }

        void TestLogSink::SinkMessage(Log::LogMessage &inMessage)
        {
            m_Message.push_back(inMessage);
        }

        bool TestLogSink::WantsLogMessage(Log::LogLevel inLevel)
        {
            return (std::find(m_WantsLevels.begin(), m_WantsLevels.end(), inLevel) != m_WantsLevels.end());
        }

        void TestLogSink::SetWantsLogLevels(const WantsLogLevelsVector &inLevels)
        {
            m_WantsLevels = inLevels;
        }

        bool TestLogSink::PopMessage(Log::LogMessage &inMessage)
        {
            if (m_Message.empty())
            {
                return false;
            }

            inMessage = m_Message.front();
            m_Message.pop_front();
            return true;
        }

        bool TestLogSink::NoMessages() const
        {
            return m_Message.empty();
        }

        void TestLogSink::FlushMessages()
        {
            m_Message.clear();
        }

        bool TestLogSink::validateMessage(const Log::LogMessage &inExpected, const IgnoreMsgKeys &inIgnoreKeys, int skipMessages)
        {
            Log::LogMessage message;
            if (m_Message.size() <= skipMessages)
            {
                std::cout << "Not enough messages to validate." << std::endl;
                return false;
            }

            for (int x = 0; x < skipMessages; x++)
            {
                m_Message.pop_front();
            }

            message = m_Message.front();
            m_Message.pop_front();

            bool retValue = true;

            //as part of the test we want to find the difference between the 2 maps
            for (auto const &it : inExpected)
            {
                if (message.find(it.first) == message.end())
                {
                    retValue = false;
                    break;
                }

                if (it.second != message[it.first])
                {
                    retValue = false;
                    break;
                }
            }

            //check for extra keys only if we validated the expected message
            if (retValue)
            {
                //now run the reverse to find extra keys
                for (auto const &it : message)
                {
                    //if it's in the ignore list then skip it
                    if (std::find(inIgnoreKeys.begin(), inIgnoreKeys.end(), it.first) != inIgnoreKeys.end())
                    {
                        continue;
                    }

                    if (inExpected.find(it.first) == inExpected.end())
                    {
                        retValue = false;
                        continue;
                    }
                }
            }

            if (retValue == false)
            {
                //ok pritnt hem out so we can see what was different
                std::cout << "Expected:" << std::endl
                          << "{" << std::endl;
                for (auto const &it : inExpected)
                {
                    std::cout << "   " << it.first << " : " << it.second << std::endl;
                }
                std::cout << "}" << std::endl;

                std::cout << "Actual:" << std::endl
                          << "{" << std::endl;
                for (auto const &it : message)
                {
                    std::cout << "   " << it.first << " : " << it.second << std::endl;
                }
                std::cout << "}" << std::endl;
            }

            return retValue;
        }
    }
}