// Copyright 2020 The v8App Authors. All rights reserved.
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
        class ILogSink
        {
        public:
            virtual ~ILogSink(){}

            virtual void SinkMessage(LogMessage &inMessage) = 0;
            virtual bool WantsLogMessage(LogLevel inLevel) = 0;

            const std::string GetName() const { return m_Name; };
            void SetName(const std::string &inName) { m_Name = inName; };

        protected:
            std::string m_Name;
        };
    } // namespace Log
} // namespace v8App
#endif