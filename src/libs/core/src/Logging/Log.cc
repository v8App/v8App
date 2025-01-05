// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <type_traits>

#include "Logging/ILogSink.h"
#include "Logging/Log.h"

namespace v8App
{
    namespace Log
    {

        const std::string LogLevelToString(LogLevel inLevel)
        {
            switch (inLevel)
            {
            case LogLevel::Fatal:
                return "Fatal";
            case LogLevel::Off:
                return "Off";
            case LogLevel::Error:
                return "Error";
            case LogLevel::General:
                return "General";
            case LogLevel::Warn:
                return "Warn";
            case LogLevel::Debug:
                return "Debug";
            default:
                return "Trace";
            };
        }

        std::map<std::string, std::unique_ptr<ILogSink>> Log::m_LogSinks;
        LogLevel Log::m_LogLevel = LogLevel::Error;
        bool Log::m_UseUTC = true;
        std::string Log::m_AppName = "v8App";

        void Log::SetLogLevel(LogLevel inLevel)
        {
            // Fatal can't be set.
            if (inLevel == LogLevel::Fatal)
            {
                return;
            }
            m_LogLevel = inLevel;
        }

        bool Log::AddLogSink(std::unique_ptr<ILogSink> &inSink)
        {
            auto it = m_LogSinks.find(inSink->GetName());
            if (it != m_LogSinks.end())
            {
                LogMessage message;
                message.emplace(MsgKey::Msg, "Sink with name:" + inSink->GetName() + " already exists");
                Log::Warn(message);
                return false;
            }
            m_LogSinks.emplace(inSink->GetName(), std::move(inSink));
            return true;
        }

        void Log::RemoveLogSink(const std::string &inName)
        {
            auto it = m_LogSinks.find(inName);
            if (it == m_LogSinks.end())
            {
                return;
            }

            m_LogSinks.erase(it);
        }

        ILogSink *Log::GetLogSink(const std::string &inName)
        {
            auto it = m_LogSinks.find(inName);
            if (it == m_LogSinks.end())
            {
                return nullptr;
            }
            return it->second.get();
        }

        std::string Log::GenerateISO8601Time(bool inUTC)
        {
            auto now = std::chrono::system_clock::now();
            auto ts = std::chrono::system_clock::to_time_t(now);
            std::stringstream sStream;
            if (inUTC)
            {
                sStream << std::put_time(gmtime(&ts), "%FT%TZ");
            }
            else
            {
                sStream << std::put_time(localtime(&ts), "%FT%T");
            }
            return sStream.str();
        }

        void Log::Error(LogMessage &inMessage)
        {
            if (m_LogLevel >= LogLevel::Error)
            {
                InternalLog(inMessage, LogLevel::Error);
            }
        }

        void Log::Error(std::string inMessage)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Error(msg);
        }

        void Log::General(LogMessage &inMessage)
        {
            if (m_LogLevel >= LogLevel::General)
            {
                InternalLog(inMessage, LogLevel::General);
            }
        }

        void Log::General(std::string inMessage)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            General(msg);
        }

        void Log::Warn(LogMessage &inMessage)
        {
            if (m_LogLevel >= LogLevel::Warn)
            {
                InternalLog(inMessage, LogLevel::Warn);
            }
        }

        void Log::Warn(std::string inMessage)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Warn(msg);
        }

        void Log::Debug(LogMessage &inMessage)
        {
            if (m_LogLevel >= LogLevel::Debug)
            {
                InternalLog(inMessage, LogLevel::Debug);
            }
        }

        void Log::Debug(std::string inMessage)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Debug(msg);
        }

        void Log::Trace(LogMessage &inMessage)
        {
            if (m_LogLevel >= LogLevel::Trace)
            {
                InternalLog(inMessage, LogLevel::Trace);
            }
        }

        void Log::Trace(std::string inMessage)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Trace(msg);
        }

        void Log::Fatal(LogMessage &inMessage)
        {
            // Fatal always logs a message
            InternalLog(inMessage, LogLevel::Fatal);
        }

        void Log::Fatal(std::string inMessage)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Fatal(msg);
        }

        void Log::General(LogMessage &inMessage, std::string File, std::string Function, int Line)
        {
            if (m_LogLevel >= LogLevel::General)
            {
                InternalLog(inMessage, LogLevel::General, File, Function, Line);
            }
        }

        void Log::General(std::string inMessage, std::string File, std::string Function, int Line)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            General(msg, File, Function, Line);
        }

        void Log::Error(LogMessage &inMessage, std::string File, std::string Function, int Line)
        {
            if (m_LogLevel >= LogLevel::Error)
            {
                InternalLog(inMessage, LogLevel::Error, File, Function, Line);
            }
        }

        void Log::Error(std::string inMessage, std::string File, std::string Function, int Line)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Error(msg, File, Function, Line);
        }

        void Log::Warn(LogMessage &inMessage, std::string File, std::string Function, int Line)
        {
            if (m_LogLevel >= LogLevel::Warn)
            {
                InternalLog(inMessage, LogLevel::Warn, File, Function, Line);
            }
        }

        void Log::Warn(std::string inMessage, std::string File, std::string Function, int Line)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Warn(msg, File, Function, Line);
        }

        void Log::Debug(LogMessage &inMessage, std::string File, std::string Function, int Line)
        {
            if (m_LogLevel >= LogLevel::Debug)
            {
                InternalLog(inMessage, LogLevel::Debug, File, Function, Line);
            }
        }

        void Log::Debug(std::string inMessage, std::string File, std::string Function, int Line)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Debug(msg, File, Function, Line);
        }

        void Log::Trace(LogMessage &inMessage, std::string File, std::string Function, int Line)
        {
            if (m_LogLevel >= LogLevel::Trace)
            {
                InternalLog(inMessage, LogLevel::Trace, File, Function, Line);
            }
        }

        void Log::Trace(std::string inMessage, std::string File, std::string Function, int Line)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Trace(msg, File, Function, Line);
        }

        void Log::Fatal(LogMessage &inMessage, std::string File, std::string Function, int Line)
        {
            // Fata always logs a message
            InternalLog(inMessage, LogLevel::Fatal, File, Function, Line);
        }

        void Log::Fatal(std::string inMessage, std::string File, std::string Function, int Line)
        {
            LogMessage msg = {{MsgKey::Msg, inMessage}};
            Fatal(msg, File, Function, Line);
        }

        void Log::Shutdown()
        {
            for(auto &it: m_LogSinks)
            {
                it.second->Close();
            }
        }

        void Log::InternalLog(LogMessage &inMessage, LogLevel inLevel, std::string File, std::string Function, int Line)
        {
            inMessage.emplace(MsgKey::File, File);
            inMessage.emplace(MsgKey::Function, Function);
            inMessage.emplace(MsgKey::Line, std::to_string(Line));
            InternalLog(inMessage, inLevel);
        }

        void Log::InternalLog(LogMessage &inMessage, LogLevel inLevel)
        {
            // add in the timestamp
            inMessage.emplace(MsgKey::TimeStamp, GenerateISO8601Time(m_UseUTC));
            // add the app name
            inMessage.emplace(MsgKey::AppName, m_AppName);
            // Add the log level
            inMessage.emplace(MsgKey::LogLevel, LogLevelToString(inLevel));

            for (auto &it : m_LogSinks)
            {
                // sinks always get fatal message whther they want them or not
                if (it.second->WantsLogMessage(inLevel) == false && inLevel != LogLevel::Fatal)
                {
                    continue;
                }
                it.second->SinkMessage(inMessage);
            }

            // if there were no sinks then send the message to the std::cerr
            if (m_LogSinks.empty())
            {
                std::cerr << m_AppName << " Log {" << std::endl;
                for (auto &it : inMessage)
                {
                    // skip the app name key
                    if (it.first == MsgKey::AppName)
                    {
                        continue;
                    }
                    std::cerr << "   " << it.first << ":" << it.second << std::endl;
                }
                std::cerr << "}" << std::endl;
            }
        }

    } // namespace Log
} // namespace v8App