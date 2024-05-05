// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _LOG_H_
#define _LOG_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace v8App
{
    namespace Log
    {
        class ILogSink;

        enum LogLevel
        {
            Fatal = -1,
            Off,
            Error,
            General,
            Warn,
            Debug,
            Trace
        };

        const std::string LogLevelToString(LogLevel inLevel);

        typedef std::map<std::string, std::string> LogMessage;

        namespace MsgKey
        {
            const std::string LogLevel = "LogLevel";
            const std::string Msg = "Message";
            const std::string TimeStamp = "Timestamp";
            const std::string AppName = "AppNmae";
            const std::string File = "File";
            const std::string Line = "Line";
            const std::string Function = "Function";
            const std::string StackTrace = "StackTrace";
            const std::string RootPath = "rootPath";
        } // namespace MsgKey

        class Log
        {
        public:
            //StandardKeys for messages
            static void SetAppName(const std::string &inAppName)
            {
                if (inAppName.empty() == false)
                {
                    m_AppName = inAppName;
                }
            }
            static std::string GetAppName() { return m_AppName; }

            static void SetLogLevel(LogLevel inLevel);
            static LogLevel GetLogLevel() { return m_LogLevel; }
            
            static void UseUTC(bool inUse) { m_UseUTC = inUse; }
            static bool IsUsingUTC() { return m_UseUTC; }

            static bool AddLogSink(std::unique_ptr<ILogSink>& inSink);
            static void RemoveLogSink(const std::string &inName);

            //You should not store the pointer returned by this method as it's managed bya std::unique_ptr
            //and could be destroyed at any time.
            static ILogSink* GetLogSink(const std::string& inName);

            static std::string GenerateISO8601Time(bool inUTC);

            static void General(LogMessage &inMessage);
            static void Error(LogMessage &inMessage);
            static void Warn(LogMessage &inMessage);
            static void Debug(LogMessage &inMessage);
            static void Trace(LogMessage &inMessage);
            static void Fatal(LogMessage &inMessage);

            static void General(LogMessage &inMessage, std::string File, std::string Function, int Line);
            static void Error(LogMessage &inMessage, std::string File, std::string Function, int Line);
            static void Warn(LogMessage &inMessage, std::string File, std::string Function, int Line);
            static void Debug(LogMessage &inMessage, std::string File, std::string Function, int Line);
            static void Trace(LogMessage &inMessage, std::string File, std::string Function, int Line);
            static void Fatal(LogMessage &inMessage, std::string File, std::string Function, int Line);

        protected:
            static void InternalLog(LogMessage &inMessage, LogLevel inLevel, std::string File, std::string Function, int Line);
            static void InternalLog(LogMessage &inMessage, LogLevel inLevel);

            static std::map<std::string, std::unique_ptr<ILogSink>> m_LogSinks;
            static LogLevel m_LogLevel;
            static bool m_UseUTC;
            static std::string m_AppName;
        };
    } // namespace Log
} // namespace v8App
#endif