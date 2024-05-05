// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <sstream>
#include <vector>
#include <iostream>

#include "Logging/LogJSONFile.h"

namespace v8App
{
    namespace Log
    {
        LogJSONFile::LogJSONFile(std::string inName, std::filesystem::path inFilePath)
            : ILogSink(inName), m_LogFile(inFilePath)
        {
        }

        LogJSONFile::~LogJSONFile()
        {
        }

        void LogJSONFile::SinkMessage(LogMessage &InMessage)
        {
            std::stringstream strMessage;
            std::vector<std::string> keys;

            for (auto it : InMessage)
            {
                keys.push_back(it.first);
            }
            std::sort(keys.begin(), keys.end());

            strMessage << "{";
            for (auto key : keys)
            {
                strMessage << "\"" << key << "\":\"" << InMessage[key] << "\",";
            }
            std::string outMessage = strMessage.str().substr(0, strMessage.str().length() - 1) + "}\n";
            if(OpenFile() == false)
            {
                std::cerr << "Failed to open JSON log file: \"" << m_LogFile.string() << "\"" <<std::endl
                << "Log: " << outMessage;
                return;
            }

            m_FileHandle << outMessage;
        }

        bool LogJSONFile::WantsLogMessage(LogLevel inLevel)
        {
            // we  take them all except off
            return inLevel != LogLevel::Off;
        }

        bool LogJSONFile::OpenFile()
        {
            if (m_FileHandle.is_open() && m_FileHandle.bad() == false && m_FileHandle.fail() == false)
            {
                return true;
            }
            m_FileHandle.close();
            if (m_LogFile.empty())
            {
                return false;
            }

            m_FileHandle.open(m_LogFile, std::ios::app | std::ios::ate);
            if (m_FileHandle.is_open() == false || m_FileHandle.bad() || m_FileHandle.fail())
            {
                m_FileHandle.close();
                return false;
            }
           return true;
        }
    }
}