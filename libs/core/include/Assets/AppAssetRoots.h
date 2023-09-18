// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_MODULESLOADER_H_
#define _JS_MODULESLOADER_H_

#include <unordered_map>
#include <filesystem>
#include <map>

#include "Utils/VersionString.h"

/**
 * Genral layout of the app's structure
 *
 * app
 *  |- js - general app js files all module files
 *  |- modules - third party pure js modules
 *  |   |- <module name>
 *  |       |- <module version>
 *  |           | - native
 *  |           |   |- <platform name>
 *  |           |       | - <shared and/or static lib> #Future maybe support compiling
 *  |           | - js
 *  |           | - resources
 *  |- resources
 *
 *
 * Flow for discovery in development
 * App path is set
 * Spin a task for js, modules and plugins to get the root paths
 * As scan each module version spin a task to scan the folders figure out latest version of module if more than one installed for default
 * Each task creates the modules root paths to use for loading modules when requested
 *
 * Production flow when the app is generated only modules and resources that are imported are linked in an a static map is compiled in.
 *
 *
 * package.json
 * The module.json file specifies info about the module for the package manager but not used by the module system itself
 * Format:
 * {
 *  "name":"<module name>",
 *  "version":"<version>",
 *  "dependencies":[
 *      {
 *          "name":"<module name>",
 *          "version":"<version specifier>",
 *          "url":"<module package url>",
 *          "checksum":"<sha256>"
 *      },...
 *  ],
 *  "devDependencies":[
 *      {
 *          "name":"<module name>",
 *          "version":"<version specifier>",
 *          "url":"<module package url>",
 *          "checksum":"<sha256>"
 *      },...
 *  ]
 * }
 *
 */
namespace v8App
{
    namespace Assets
    {
        static const std::string c_RootJS = "js";
        static const std::string c_RootResource = "resource";
        static const std::string c_RootModules = "modules";

        class AppAssetRoots
        {
        public:
            AppAssetRoots() {}
            ~AppAssetRoots() {}

            /**
             * Sets the applications root path and then scans the directory looking for the js directory and modules directory
             * scanning the modules directory to get valid modules and their versions tracking the latest version.
             */
            bool SetAppRootPath(std::string inAppRootPath);
            bool SetAppRootPath(std::filesystem::path inAppRootPath);
            std::filesystem::path GetAppRoot() const { return m_AppRoot; }
            /**
             * Adds a modules root path making it faster to build the path to the modules file
             */
            bool AddModuleRootPath(std::string inModuleNmae, std::string inPath);
            bool AddModuleRootPath(std::string inModuleNmae, std::filesystem::path inPath);
            /**
             * Finds the modules root path
             */
            std::filesystem::path FindModuleRootPath(std::string inModule);
            /**
             * Remove the modules root path
             */
            void RemoveModuleRootPath(std::string inModuleName);
            /**
             * Sets the modules latest version
             */
            void SetModulesLatestVersion(std::string inModuleName, Utils::VersionString &inVersion);
            /**
             * Gets the modules latest version used when no version is specified on an import
             */
            Utils::VersionString GetModulesLatestVersion(const std::string &inModuleName);
            /**
             * Removes modules latest version
            */
           void RemoveModulesLatestVersion(std::string inModule);
            /**
             * Check to see if a relative path escaped out of the specified root path
             */
            bool DidPathEscapeRoot(std::filesystem::path inRootPath, std::filesystem::path inScriptPath);

            /**
             * Make a relative path from the set app root. If the relative path escapes the app root then an empty path is returned.
             */
            std::filesystem::path MakeRelativePathToAppRoot(std::string inPath);
            std::filesystem::path MakeRelativePathToAppRoot(std::filesystem::path inPath);
            /**
             * Make a relative path from the specified root. If the relative path escapes the specified root then an empty path is returned.
             */
            std::filesystem::path MakeRelativePathToRoot(std::string inPath, std::string inRoot);
            std::filesystem::path MakeRelativePathToRoot(std::filesystem::path inPath, std::filesystem::path inRoot);
            /**
             * Creates an absolute path from a relative path. If the path escapes the app root it returns an empty path.
             * An absolute path can be passed as well and it'll just check that it didn't escape.
             */
            std::filesystem::path MakeAbsolutePathChecked(std::string inPath);
            std::filesystem::path MakeAbsolutePathChecked(std::filesystem::path inPath);
            /**
             * Creates an absolute path from a relative path.
             * An absolute path can be passed as well and it'll just check that it didn't escape.
             */
            std::filesystem::path MakeAbsolutePath(std::string inPath);
            std::filesystem::path MakeAbsolutePath(std::filesystem::path inPath);

            /**
             * Normalizes the path sperator to a forward slash
            */
            std::string NormalizePathSeperator(const std::filesystem::path& inPath );

        protected:
            /**
             * Scans the app root for the js direcotry and modules directory and then add the modules roots and latest version
             */
            bool FindAssetRoots(std::filesystem::path inRootPath);

            std::map<std::string, std::filesystem::path> m_ModuleRoots;
            std::map<std::string, Utils::VersionString> m_ModuleLatestVersion;
            std::filesystem::path m_AppRoot;
        };

        using AppAssetRootsWeakPtr = std::weak_ptr<class AppAssetRoots>;
        using AppAssetRootsSharedPtr = std::shared_ptr<AppAssetRoots>;
    }
}
#endif //_JS_MODULESLOADER_H_
