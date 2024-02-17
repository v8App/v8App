// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __LOG_JSON_FILE_H__
#define __LOG_JSON_FILE_H__

#include <filesystem>
#include <fstream>

#include "Logging/ILogSink.h"

namespace v8App
{
    namespace Log
    {
        class LogJSONFile : public ILogSink
        {
        public:
            LogJSONFile(std::string inName, std::filesystem::path inFilePath);
            virtual ~LogJSONFile();

            virtual void SinkMessage(LogMessage &InMessage) override;
            virtual bool WantsLogMessage(LogLevel inLevel) override;            

            std::filesystem::path GetFilePath() { return m_LogFile; }
        protected:
            bool OpenFile();

            std::fstream m_FileHandle;
            std::filesystem::path m_LogFile;
        };
    }
}
#endif //__LOG_JSON_FILE_H__