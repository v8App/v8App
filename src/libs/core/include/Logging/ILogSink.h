// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _ILOG_SINK_H_
#define _ILOG_SINK_H_

#include <map>

#include "Logging/Log.h"

namespace v8App
{
    namespace Log
    {
        /**
         * Interface class for the log system sinks for messages.
        */
        class ILogSink
        {
        public:
            ILogSink(const std::string& inName):m_Name(inName){}
            virtual ~ILogSink(){}

            virtual void SinkMessage(LogMessage &inMessage) = 0;
            virtual bool WantsLogMessage(LogLevel inLevel) = 0;

            /**
             * Perform any final work for the log and close it.
            */
            virtual void Close() = 0;

            const std::string GetName() const { return m_Name; };
            void SetName(const std::string &inName) { m_Name = inName; };

        protected:
            std::string m_Name;
        };
    } // namespace Log
} // namespace v8App
#endif