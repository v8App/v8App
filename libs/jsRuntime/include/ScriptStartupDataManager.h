// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __SCRIPT_STARTUP_DATA_MANAGER__
#define __SCRIPT_STARTUP_DATA_MANAGER__

#include <filesystem>
#include <map>

#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {

        using ScriptCacheMap = std::map<std::filesystem::path, std::string>;

        class ScriptStartupDataManager
        {
        public:
            ScriptStartupDataManager() {}
            ~ScriptStartupDataManager() {}

            //Initialize the startup data for v8 using the passed in file for the data.
            //In release one it expects to find the startup data embedded in the binary
            static bool InitializeStartupData(std::filesystem::path inStartupDataFile);

            //Initialize the icu data if it's being used.
            static bool InitializeICUData(std::filesystem::path inICUDataFile);

            //loads rhe file either from a cache or if not in cache reads it from the file.
            //the cache could be preseeded by embedding it in the binary
            static bool LoadScriptFile(std::filesystem::path inFIleName, std::string &outContents);

            //this allows the script to be loaded into cache form another source like across the network
            static void SetCachedFile(std::filesystem::path inFileName, std::string inScript);

            static void ClearCache();

        protected:
            static std::string CheckCacheForFile(std::filesystem::path inFilePath);

        private:
            static v8::StartupData *GetStartupData();

            static bool ReadScriptFile(std::filesystem::path inFileName, std::string &outContents);

            static ScriptCacheMap s_ScriptCache;

            ScriptStartupDataManager(const ScriptStartupDataManager &) = delete;
            ScriptStartupDataManager(ScriptStartupDataManager &&) = delete;
            ScriptStartupDataManager &operator=(const ScriptStartupDataManager &) = delete;
            ScriptStartupDataManager &operator=(ScriptStartupDataManager &&) = delete;
        };
    }
}
#endif
