// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <sstream>
#include <fstream>

#include "Logging/LogMacros.h"
#include "Assets/AppAssetRoots.h"
#include "Assets/BinaryAsset.h"
#include "Assets/TextAsset.h"
#include "Utils/Format.h"
#include "Utils/Paths.h"

#include "CodeCache.h"
#include "JSContext.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        CodeCache::CodeCache(JSAppSharedPtr inApp) : m_App(inApp)
        {
        }

        CodeCache::~CodeCache()
        {
            m_App.reset();
        }

        V8ScriptSourceUniquePtr CodeCache::LoadScriptFile(std::filesystem::path inFilePath, V8Isolate *inIsolate)
        {
            if (inFilePath.string() == "")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Empty file name passed for a script file"));
                LOG_ERROR(msg);
                return nullptr;
            }

            std::string ext = inFilePath.extension().string();

            if (ext != ".js" && ext != ".mjs")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Unsupported file extension passed, only .mjs, .js allowed. File: {}", inFilePath));
                LOG_ERROR(msg);
                return nullptr;
            }

            std::filesystem::path fileRoot = Utils::MakeRelativePathToRoot(inFilePath, m_App->GetAppRoots()->GetAppRoot());
            auto it = fileRoot.begin();
            if (it->string() != Assets::c_RootJS && it->string() != Assets::c_RootModules)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Script file is not in the js or modules directories. File: {}", inFilePath));
                LOG_ERROR(msg);
                return nullptr;
            }

            if (std::filesystem::exists(inFilePath) == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("File does not exists. File: {}", inFilePath));
                LOG_ERROR(msg);
                return nullptr;
            }

            std::filesystem::path cachePath = GenerateCachePath(inFilePath);
            if (cachePath.empty())
            {
                // no log generate emits one
                return nullptr;
            }

            std::filesystem::file_time_type jsModTime = std::filesystem::last_write_time(inFilePath);
            std::filesystem::file_time_type jsccModTime;

            ScriptCacheInfo *cacheInfo = GetCachedScript(inFilePath.generic_string());
            // no entry yet so build one
            if (cacheInfo == nullptr)
            {
                cacheInfo = CreateCacheInfo(inFilePath.generic_string());
                if (std::filesystem::exists(cachePath))
                {
                    jsccModTime = std::filesystem::last_write_time(cachePath);
                    if (jsccModTime >= jsModTime)
                    {
                        if (ReadCachedDataFile(inFilePath, cacheInfo) == false)
                        {
                            // no log ReadCachedDataFile emits one
                            return nullptr;
                        }
                    }
                    cacheInfo->m_LastCompiled = jsccModTime;
                }
            }

            if (cacheInfo->m_LastCompiled < jsModTime)
            {
                delete cacheInfo->m_Compiled;
                cacheInfo->m_Compiled = nullptr;
                cacheInfo->m_CompiledLength = 0;

                if (ReadScriptFile(inFilePath, cacheInfo) == false)
                {
                    return nullptr;
                }
            }

            V8LString sourceStr = JSUtilities::StringToV8(inIsolate, cacheInfo->m_SourceStr);
            V8LString fileStr = JSUtilities::StringToV8(inIsolate, cacheInfo->m_FilePath.generic_string());
            V8ScriptCachedData *cache = nullptr;
            if (cacheInfo->m_Compiled != nullptr)
            {
                cache = new V8ScriptCachedData(cacheInfo->m_Compiled, cacheInfo->m_CompiledLength, V8ScriptCachedData::BufferNotOwned);
            }
            V8ScriptOrigin origin(inIsolate, fileStr, 0, 0, false, -1, V8LValue(), false, false, true);
            return std::make_unique<V8ScriptSource>(sourceStr, origin, cache);
        }

        bool CodeCache::HasCodeCache(std::filesystem::path inFilePath)
        {
            ScriptCacheInfo* info = GetCachedScript(inFilePath.generic_string());
            if(info == nullptr)
            {
                return false;
            }
            return info->m_Compiled != nullptr;
        }

        bool CodeCache::SetCodeCache(std::filesystem::path inFilePath, V8ScriptCachedData *inCachedData)
        {
            if (inCachedData == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("SetCodeCache passed a nullptr for cached data"));
                LOG_ERROR(msg);
                return false;
            }
            ScriptCacheInfo *info = GetCachedScript(inFilePath.generic_string());
            if (info == nullptr)
            {
                // we don't have the cached script for some reason so construct one.
                info = CreateCacheInfo(inFilePath.generic_string());
                if (info == nullptr)
                {
                    // CreateCacheInfo emits a log
                    return false;
                }
            }

            if (WriteCacheDataToFile(info->m_CachedFilePath, inCachedData->data, inCachedData->length) == false)
            {
                // WriteCacheDataToFile emits a log
                return false;
            }

            if (info->m_Compiled != nullptr)
            {
                delete info->m_Compiled;
            }

            info->m_Compiled = new uint8_t[inCachedData->length];
            memcpy(info->m_Compiled, inCachedData->data, inCachedData->length);
            info->m_LastCompiled = std::filesystem::last_write_time(info->m_CachedFilePath);
            return true;
        }

        CodeCache::ScriptCacheInfo *CodeCache::GetCachedScript(std::string inFile)
        {
            auto it = m_ScriptCache.find(inFile);
            if (it == m_ScriptCache.end())
            {
                return nullptr;
            }
            return it->second.get();
        }

        CodeCache::ScriptCacheInfo *CodeCache::CreateCacheInfo(std::string inFilePath)
        {
            std::unique_ptr<CodeCache::ScriptCacheInfo> info = std::make_unique<CodeCache::ScriptCacheInfo>();
            info->m_FilePath = inFilePath;

            info->m_CachedFilePath = GenerateCachePath(inFilePath);
            if (info->m_CachedFilePath.empty())
            {
                // GenerateCachePath emits a log
                return nullptr;
            }

            // we'll assume that the srouce in the file hasn't changed sinne the cache was created
            if (ReadScriptFile(inFilePath, info.get()) == false)
            {
                // ReadScriptFile emits a log
                return nullptr;
            }
            if (m_ScriptCache.insert(std::make_pair(inFilePath, std::move(info))).second == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to insert the cache info"));
                LOG_ERROR(msg);
                return nullptr;
            }
            return m_ScriptCache[inFilePath].get();
        }

        std::filesystem::path CodeCache::GenerateCachePath(std::filesystem::path inFilePath)
        {
            std::filesystem::path cachePath = m_App->GetAppRoots()->MakeRelativePathToAppRoot(inFilePath);
            auto it = cachePath.begin();
            if (it->string() != "js" && it->string() != "modules")
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Script file is not in the js or modules directories. File: {}", inFilePath));
                LOG_ERROR(msg);
                return std::filesystem::path();
            }
            cachePath = m_App->GetAppRoots()->GetAppRoot() / std::filesystem::path(".code_cache") / cachePath;
            cachePath = cachePath.replace_extension(std::string("jscc"));
            return cachePath;
        }

        bool CodeCache::ReadScriptFile(std::filesystem::path inFilePath, ScriptCacheInfo *inInfo)
        {
            if (inInfo == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("RreadScriptFile passed a nullptr for info."));
                LOG_ERROR(msg);
                return false;
            }
            Assets::TextAsset file(inFilePath);

            if (file.Exists() == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("File doesn't exist: {}", inFilePath));
                LOG_ERROR(msg);
                return false;
            }

            if (file.ReadAsset() == false)
            {
                return false;
            }
            inInfo->m_SourceStr = file.GetContent();

            return true;
        }

        bool CodeCache::WriteCacheDataToFile(std::filesystem::path inCachePath, const uint8_t *inData, int inDataLength)
        {
            if (inData == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("WriteCacheDataToFile passed a nllptr for data"));
                LOG_ERROR(msg);
                return false;
            }
            // need to make sure the direcotires to the file are created which they won't be for a new file
            std::filesystem::path dir_path = inCachePath.parent_path();
            if (std::filesystem::exists(dir_path) == false && dir_path.empty() == false)
            {
                // would check if it returns true/false except it's not supposed to error on existing but does.
                std::filesystem::create_directories(dir_path);
                // check to see that it was created or return an error
                if (std::filesystem::exists(dir_path) == false)
                {
                    Log::LogMessage msg;
                    msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to create the cache directory. Path: {}", dir_path));
                    LOG_ERROR(msg);
                    return false;
                }
            }

            Assets::BinaryAsset file(inCachePath);
            std::vector<uint8_t> vecData(inData, inData + inDataLength);
            if (file.SetContent(vecData) == false)
            {
                return false;
            }
            return file.WriteAsset();
        }

        bool CodeCache::ReadCachedDataFile(std::filesystem::path inCachePath, ScriptCacheInfo *inInfo)
        {
            if (inInfo == nullptr)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("ReadCachedDataFile passed a nllptr for info"));
                LOG_ERROR(msg);
                return false;
            }
            Assets::BinaryAsset file(inCachePath);
            if (file.Exists() == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Cached data file doesn't exist: {}", inCachePath));
                LOG_ERROR(msg);
                return false;
            }

            if (file.ReadAsset() == false)
            {
                return false;
            }
            const std::vector<uint8_t> &buffer = file.GetContent();
            if (buffer.size() == 0)
            {
                return true;
            }

            inInfo->m_CompiledLength = buffer.size();
            if (inInfo->m_Compiled != nullptr)
            {
                delete inInfo->m_Compiled;
            }
            inInfo->m_Compiled = new uint8_t[inInfo->m_CompiledLength];
            memcpy(inInfo->m_Compiled, buffer.data(), inInfo->m_CompiledLength);

            return true;
        }

    }
}