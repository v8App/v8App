// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef ___CODE_CACHE__
#define ___CODE_CACHE__

#include <filesystem>
#include <map>

#include "Assets/AppAssetRoots.h"

#include "V8Types.h"
#include "JSApp.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Manages scripts neing loaded using code cache to speed up compile times
         *
         * TODO: Implement storing the CachedDataVersionTag along side the code cache data in the file.
         */
        class CodeCache
        {
        public:
            struct ScriptCacheInfo
            {
                ~ScriptCacheInfo()
                {
                    if (m_Compiled != nullptr)
                    {
                        delete m_Compiled;
                    }
                    m_Compiled = nullptr;
                    m_CompiledLength = 0;
                }

                std::filesystem::file_time_type m_LastCompiled;
                std::string m_SourceStr;
                uint8_t *m_Compiled = nullptr;
                int m_CompiledLength = 0;
                std::filesystem::path m_FilePath;
                std::filesystem::path m_CachedFilePath;
            };

            CodeCache(JSAppSharedPtr inApp);
            ~CodeCache();

            // loads rhe file either from a cache or if not in cache reads it from the file.
            // the cache could be preseeded by embedding it in the binary
            V8ScriptSourceUniquePtr LoadScriptFile(std::filesystem::path inFilePath, V8Isolate *inIsolate);
            bool HasCodeCache(std::filesystem::path inFilePath);
            bool SetCodeCache(std::filesystem::path inFilePath, V8ScriptCachedData *inCachedData);

        protected:
            ScriptCacheInfo *GetCachedScript(std::string inFilePath);
            ScriptCacheInfo *CreateCacheInfo(std::string inFilePath);

            std::filesystem::path GenerateCachePath(std::filesystem::path inFilePath);

            bool ReadScriptFile(std::filesystem::path inFilePath, ScriptCacheInfo *inInfo);
            bool WriteCacheDataToFile(std::filesystem::path inCachePath, const uint8_t *inData, int inDataLength);
            bool ReadCachedDataFile(std::filesystem::path inCachePath, ScriptCacheInfo *inInfo);

            using ScriptCacheMap = std::map<std::filesystem::path, std::unique_ptr<ScriptCacheInfo>>;

            ScriptCacheMap m_ScriptCache;
            JSAppSharedPtr m_App;

            CodeCache(const CodeCache &) = delete;
            CodeCache(CodeCache &&) = delete;
            CodeCache &operator=(const CodeCache &) = delete;
            CodeCache &operator=(CodeCache &&) = delete;
        };
    }
}
#endif //___CODE_CACHE__
