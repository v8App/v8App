// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>
#include <regex>

#include "Assets/AppAssetRoots.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"

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

        bool AppAssetRoots::DidPathEscapeRoot(std::filesystem::path inRootPath, std::filesystem::path inScriptPath)
        {
            std::filesystem::path rootCanonical = std::filesystem::weakly_canonical(inRootPath).make_preferred();
            std::filesystem::path fileCanonical = std::filesystem::weakly_canonical(inScriptPath).make_preferred();

            return !fileCanonical.string().starts_with(rootCanonical.string());
        }

        std::filesystem::path AppAssetRoots::MakeRelativePathToAppRoot(std::string inPath)
        {
            std::filesystem::path path(inPath);
            return MakeRelativePathToRoot(path, m_AppRoot);
        }

        std::filesystem::path AppAssetRoots::MakeRelativePathToAppRoot(std::filesystem::path inPath)
        {
            return MakeRelativePathToRoot(inPath, m_AppRoot);
        }

        std::filesystem::path AppAssetRoots::MakeRelativePathToRoot(std::string inPath, std::string inRoot)
        {
            std::filesystem::path path(inPath);
            std::filesystem::path root(inRoot);
            return MakeRelativePathToRoot(path, root);
        }

        std::filesystem::path AppAssetRoots::MakeRelativePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot)
        {
            if (inPath.is_absolute())
            {
                if (DidPathEscapeRoot(inRoot, inPath))
                {
                    return std::filesystem::path();
                }
                inPath = inPath.lexically_relative(inRoot);
            }

            return std::filesystem::path(NormalizePathSeperator(inPath));
        }

        std::filesystem::path AppAssetRoots::MakeAbsolutePathChecked(std::string inPath)
        {
            std::filesystem::path path(inPath);
            return MakeAbsolutePathChecked(path);
        }
        std::filesystem::path AppAssetRoots::MakeAbsolutePathChecked(std::filesystem::path inPath)
        {
            std::filesystem::path path = MakeAbsolutePath(inPath);
            if (path.empty())
            {
                return path;
            }
            if (DidPathEscapeRoot(m_AppRoot, path))
            {
                return std::filesystem::path();
            }
            return path;
        }

        std::filesystem::path AppAssetRoots::MakeAbsolutePath(std::string inPath)
        {
            std::filesystem::path path(inPath);
            return MakeAbsolutePath(path);
        }

        std::filesystem::path AppAssetRoots::MakeAbsolutePath(std::filesystem::path inPath)
        {
            if (inPath.is_absolute())
            {
                return NormalizePathSeperator(inPath);
            }

            return std::filesystem::absolute(std::filesystem::path(NormalizePathSeperator(inPath)));
        }

        std::string AppAssetRoots::NormalizePathSeperator(const std::filesystem::path &inPath)
        {
            std::string path = inPath.string();
            std::replace(path.begin(), path.end(), '\\', '/');

            return path;
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
                        std::string moduleNameStr = NormalizePathSeperator((moduleName / versionEntry.path().filename()));
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
    }
}