// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEST_LOG_SINK_H__
#define __TEST_LOG_SINK_H__

#include <deque>
#include <iostream>

#include "Logging/ILogSink.h"

namespace v8App
{
    namespace TestUtils
    {
        using WantsLogLevelsVector = std::vector<Log::LogLevel>;
        using IgnoreMsgKeys = std::vector<std::string>;

        class TestLogSink : public Log::ILogSink
        {
        public:
            TestLogSink(const std::string &inName, const WantsLogLevelsVector &inWantLevels);
            virtual ~TestLogSink() {}

            static TestLogSink *GetGlobalSink();

            virtual void SinkMessage(Log::LogMessage &inMessage);

            virtual bool WantsLogMessage(Log::LogLevel inLevel);

            virtual void Close() override {};
            void SetWantsLogLevels(const WantsLogLevelsVector &inLevels);

            bool PopMessage(Log::LogMessage &inMessage);
            bool NoMessages() const;
            void FlushMessages();

            bool ValidateMessage(const Log::LogMessage &inMessage, const IgnoreMsgKeys &inIgnoreKeys = IgnoreMsgKeys(), int skipMessages = 0);

            static const std::string GlobalTestSinkName;

        protected:
            std::deque<Log::LogMessage> m_Message;
            WantsLogLevelsVector m_WantsLevels;
        };

    }
}
#endif //__TEST_LOG_SINK_H__