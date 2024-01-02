// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>
#include <regex>

#include "Assets/AppAssetRoots.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"
#include "Utils/Paths.h"

namespace v8App
{
    namespace Assets
    {
        bool AppAssetRoots::SetAppRootPath(std::string inAppRootPath)
        {
            return SetAppRootPath(std::filesystem::path(inAppRootPath));
        }

        bool AppAssetRoots::SetAppRootPath(std::filesystem::path inAppRootPath)
        {
            if (m_AppRoot.empty())
            {
                if (std::filesystem::exists(inAppRootPath) && std::filesystem::is_directory(inAppRootPath))
                {
                    if (FindAssetRoots(inAppRootPath))
                    {
                        m_AppRoot = std::filesystem::absolute(inAppRootPath);
                        return true;
                    }
                }
            }
            return false;
        }

        bool AppAssetRoots::AddModuleRootPath(std::string inModuleName, std::string inPath)
        {
            std::filesystem::path path(inPath);
            return AddModuleRootPath(inModuleName, path);
        }

        bool AppAssetRoots::AddModuleRootPath(std::string inModuleName, std::filesystem::path inPath)
        {
            auto it = m_ModuleRoots.emplace(inModuleName, inPath);
            return it.second;
        }

        std::filesystem::path AppAssetRoots::FindModuleRootPath(std::string inModule)
        {
            auto it = m_ModuleRoots.find(inModule);
            if (it != m_ModuleRoots.end())
            {
                return it->second;
            }

            return std::filesystem::path();
        }

        void AppAssetRoots::RemoveModuleRootPath(std::string inModuleName)
        {
            m_ModuleRoots.erase(inModuleName);
        }

        void AppAssetRoots::SetModulesLatestVersion(std::string inModuleName, Utils::VersionString &inVersion)
        {
            m_ModuleLatestVersion.insert_or_assign(inModuleName, inVersion);
        }

        Utils::VersionString AppAssetRoots::GetModulesLatestVersion(const std::string &inModuleName)
        {
            auto it = m_ModuleLatestVersion.find(inModuleName);
            if (m_ModuleLatestVersion.end() == it)
            {
                return Utils::VersionString("");
            }
            return it->second;
        }

        void AppAssetRoots::RemoveModulesLatestVersion(std::string inModule)
        {
            m_ModuleLatestVersion.erase(inModule);
        }

        std::filesystem::path AppAssetRoots::MakeRelativePathToAppRoot(std::string inPath)
        {
            return MakeRelativePathToAppRoot(std::filesystem::path(inPath));
        }

        std::filesystem::path AppAssetRoots::MakeRelativePathToAppRoot(std::filesystem::path inPath)
        {
            inPath = ReplaceTokens(inPath);
            return Utils::MakeRelativePathToRoot(inPath, m_AppRoot);
        }

        std::filesystem::path AppAssetRoots::MakeAbsolutePathToAppRoot(std::string inPath)
        {
            return MakeAbsolutePathToAppRoot(std::filesystem::path(inPath));
        }

        std::filesystem::path AppAssetRoots::MakeAbsolutePathToAppRoot(std::filesystem::path inPath)
        {
            inPath = ReplaceTokens(inPath);
            return Utils::MakeAbsolutePathToRoot(inPath, m_AppRoot);
        }

        bool AppAssetRoots::FindAssetRoots(std::filesystem::path inRootPath)
        {

            bool foundJS = false;
            bool foundModules = false;
            bool foundResources = false;

            for (auto const &dirEntry : std::filesystem::directory_iterator(inRootPath))
            {
                std::string filename = dirEntry.path().filename().string();
                if (dirEntry.is_directory() && dirEntry.path().filename().string() == c_RootJS)
                {
                    foundJS = true;
                    continue;
                }
                if (dirEntry.is_directory() && dirEntry.path().filename().string() == c_RootModules)
                {
                    foundModules = true;
                    continue;
                }
                if (dirEntry.is_directory() && dirEntry.path().filename().string() == c_RootResource)
                {
                    foundResources = true;
                    continue;
                }
            }
            if (foundJS == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to find the {} directory in the app root", c_RootJS));
                LOG_ERROR(msg);
                return false;
            }
            if (foundModules == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to find the {} directory in the app root", c_RootModules));
                LOG_ERROR(msg);
                return false;
            }
            if (foundResources == false)
            {
                Log::LogMessage msg;
                msg.emplace(Log::MsgKey::Msg, Utils::format("Failed to find the {} directory in the app root", c_RootResource));
                LOG_ERROR(msg);
                return false;
            }
            // add the js folder as a module
            AddModuleRootPath(c_RootJS, inRootPath / c_RootJS);
            AddModuleRootPath(c_RootResource, inRootPath / c_RootResource);

            std::filesystem::path baseModules = inRootPath / c_RootModules;
            for (auto const &dirEntry : std::filesystem::directory_iterator(baseModules))
            {
                if (dirEntry.is_directory() == false)
                {
                    continue;
                }
                std::filesystem::path moduleName = dirEntry.path().filename();
                // modules cannot be names js or resources
                if (moduleName == c_RootJS || moduleName == c_RootResource)
                {
                    continue;
                }
                for (auto const &versionEntry : std::filesystem::directory_iterator(dirEntry.path()))
                {
                    Utils::VersionString moduleVersion(versionEntry.path().filename().string());
                    if (moduleVersion.IsVersionString())
                    {
                        std::string moduleNameStr = Utils::NormalizePath((moduleName / versionEntry.path().filename()));
                        AddModuleRootPath(moduleNameStr, versionEntry.path());
                        Utils::VersionString latest = GetModulesLatestVersion(moduleName.string());
                        if (latest.IsVersionString())
                        {
                            if (latest < moduleVersion)
                            {
                                SetModulesLatestVersion(moduleName.string(), moduleVersion);
                            }
                        }
                        else
                        {
                            SetModulesLatestVersion(moduleName.string(), moduleVersion);
                        }
                    }
                }
            }
            return true;
        }

        std::filesystem::path AppAssetRoots::ReplaceTokens(std::filesystem::path inPath)
        {
            std::string strPath = inPath.string();
            int subStrLen = -1;
            std::string tokenReplace = "";
            if (strPath.starts_with(c_AppRoot_Token))
            {
                subStrLen = 9;
            }
            else if (strPath.starts_with(c_Js_Token))
            {
                subStrLen = 4;
                tokenReplace = "js";
            }
            else if (strPath.starts_with(c_Modules_Token))
            {
                subStrLen = 9;
                tokenReplace = "modules";
            }
            else if (strPath.starts_with(c_Resources_Token))
            {
                subStrLen = 11;
                tokenReplace = "resources";
            }
            if (subStrLen == -1)
            {
                return inPath;
            }
            // we want to make sure that the slash at the begining is removed as it seems it might
            // prevent the paths from concating correctly
            inPath = std::filesystem::path(strPath.substr(subStrLen)).lexically_relative(std::filesystem::path("/"));
            if (tokenReplace != "")
            {
                inPath = std::filesystem::path(tokenReplace) / inPath;
            }
            return m_AppRoot / inPath;
        }
    }
}