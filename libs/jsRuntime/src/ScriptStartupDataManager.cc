// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <sstream>
#include <fstream>

#include "ScriptStartupdataManager.h"
#include "Logging/LogMacros.h"

namespace v8App
{
    namespace JSRuntime
    {
        ScriptCacheMap ScriptStartupDataManager::s_ScriptCache;

        bool ScriptStartupDataManager::InitializeStartupData(std::filesystem::path inStartupDataFile)
        {
            if (inStartupDataFile == "")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Empty file name passed for the startup data file");
                LOG_ERROR(msg);
                return false;
            }

#ifdef V8APP_RELEASE
            v8::StartupData *snapshot = nullptr;
            snapshot = GetStartupData();
            if (snapshot == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Got a nullptr for the startupdata in release mode. Expected it to be built in");
                LOG_ERROR(msg);
                return false;
            }

            v8::SetSnapshotDataBlob(snapshot);
#else
            if (std::filesystem::exists(inStartupDataFile) == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to find the startup data file: " + inStartupDataFile.string());
                LOG_ERROR(msg);
                return false;
            }

            v8::V8::InitializeExternalStartupDataFromFile(inStartupDataFile.string().c_str());
#endif
            return true;
        }

        bool ScriptStartupDataManager::InitializeICUData(std::filesystem::path inICUDataFile)
        {
            if (inICUDataFile == "")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Empty file name passed for the ICU data file");
                LOG_ERROR(msg);
                return false;
            }

            if (std::filesystem::exists(inICUDataFile) == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to find the ICU data file: " + inICUDataFile.string());
                LOG_ERROR(msg);
                return false;
            }

            v8::V8::InitializeICU(inICUDataFile.string().c_str());
            return true;
        }

        std::string ScriptStartupDataManager::CheckCacheForFile(std::filesystem::path inFilePath)
        {
            ScriptCacheMap::iterator it;

#ifdef V8APP_RELEASE
            it = embeddedScripts.find(inFileName);

            //first find the
            if (it != embeddedScripts.end())
            {
                return embeddedScripts[inFileName];
            }
            return std::string();
#endif

            it = s_ScriptCache.find(inFilePath);
            if (it != s_ScriptCache.end())
            {
                return s_ScriptCache[inFilePath];
            }

            return std::string();
        }

        bool ScriptStartupDataManager::LoadScriptFile(std::filesystem::path inFileName, std::string &outContents)
        {
            if (inFileName == "")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Empty file name passed for a script file");
                LOG_ERROR(msg);
                return false;
            }

            std::string ext = inFileName.extension().string();

            if (ext.empty() == false && ext != ".js" && ext != ".mjs")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Unsupported file extension passed, only .mjs, .js or no extension allowed");
                msg.emplace(Log::MsgKey::File, inFileName.string());
                LOG_ERROR(msg);
                return false;
            }

            std::filesystem::path jsPath;

            if (inFileName.extension().empty())
            {
                jsPath = inFileName;
                inFileName.replace_extension(".mjs");
                jsPath.replace_extension(".js");
            }

            outContents = CheckCacheForFile(inFileName);
            if (outContents.empty())
            {

                if (ReadScriptFile(inFileName, outContents) == false)
                {
                    outContents = CheckCacheForFile(jsPath);
                    if (outContents.empty())
                    {
                        if (ReadScriptFile(jsPath, outContents) == false)
                        {
                            return false;
                        }
                    }
                    inFileName = jsPath;
                }
            }

            s_ScriptCache.insert(std::make_pair(inFileName, outContents));
            return true;
        }

        void ScriptStartupDataManager::SetCachedFile(std::filesystem::path inFileName, std::string inScript)
        {
#ifdef V8APP_RELEASE
            return;
#endif

            ScriptCacheMap::iterator it = s_ScriptCache.find(inFileName);
            if (it == s_ScriptCache.end())
            {
                s_ScriptCache.insert(std::make_pair(inFileName, inScript));
            }
            else
            {
                s_ScriptCache[inFileName] = inScript;
            }
        }

        bool ScriptStartupDataManager::ReadScriptFile(std::filesystem::path inFileName, std::string &outContents)
        {
            std::ifstream fileStream(inFileName, std::ios::in | std::ios::binary);
            if (fileStream.is_open() == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, "Failed to open the script file: " + inFileName.string());
                LOG_ERROR(msg);
                return false;
            }

            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            fileStream.close();
            outContents = buffer.str();
            return true;
        }

        void ScriptStartupDataManager::ClearCache()
        {
            //for release can't cleat teh cache.
#if V8_RELEASE
            return;
#endif
            s_ScriptCache.clear();
        }

    }
}