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
 * Special tokens that will be replaced
 * %APP_ROOT% - willl be replaced with the app's root path set
 * %JS% will be replaced with the path to the js directory
 * %MODULES% - will be replaced with the path to the module directory
 * %RESOURCES% - will be replaced with the path to the recource directory
 * 
 * No App Asset outside of the app's root path.
 * Pats that start with / are anchroed to the app root that is set. example app root is /opt/JSApp/
 * and path is given as /js/file.js then it's abs path for the file system is /opt/JSApp/js/file.js
 * A file given as js/file.js would also have an abs of /opt/JSApp/js/file.js.
 * If a path is given as ./file.js then it'll be relative to what ever file is importing it or the app root path if not an import.
 * .. paths will work as well and the std filesystem will resolve it. Paths are checked to make sure they don't leave the app root.
 * Something like js../modules/test/test.js would resolve to /opt/JSApp/modules/test/test.js.
 */
namespace v8App
{
    namespace Assets
    {
        static const std::string c_RootJS = "js";
        static const std::string c_RootResource = "resources";
        static const std::string c_RootModules = "modules";

        static const std::string c_AppRoot_Token = "%APPROOT%";
        static const std::string c_Js_Token = "%JS%";
        static const std::string c_Resources_Token = "%RESOURCES%";
        static const std::string c_Modules_Token = "%MODULES%";

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
            bool AddModuleRootPath(std::string inModuleName, std::string inPath);
            bool AddModuleRootPath(std::string inModuleName, std::filesystem::path inPath);
            /**
             * Finds the modules root path by module name/version
             */
            std::filesystem::path FindModuleVersionRootPath(std::string inModule);
            /**
             * Finds the modules root path for latest version by module name
            */
            std::filesystem::path FindModuleLatestVersionRootPath(std::string inModule);
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
             * Make a relative path from the set app root. If the relative path escapes the app root then an empty path is returned.
             */
            std::filesystem::path MakeRelativePathToAppRoot(std::string inPath);
            std::filesystem::path MakeRelativePathToAppRoot(std::filesystem::path inPath);

            /**
             * Make a absolute path from the set app root. If the relative path escapes the app root then an empty path is returned.
             */
            std::filesystem::path MakeAbsolutePathToAppRoot(std::string inPath);
            std::filesystem::path MakeAbsolutePathToAppRoot(std::filesystem::path inPath);

        protected:
            /**
             * Scans the app root for the js direcotry and modules directory and then add the modules roots and latest version
             */
            bool FindAssetRoots(std::filesystem::path inRootPath);
            std::filesystem::path ReplaceTokens(std::filesystem::path inPath);

            std::map<std::string, std::filesystem::path> m_ModuleRoots;
            std::map<std::string, Utils::VersionString> m_ModuleLatestVersion;
            std::filesystem::path m_AppRoot;
        };

        using AppAssetRootsWeakPtr = std::weak_ptr<class AppAssetRoots>;
        using AppAssetRootsSharedPtr = std::shared_ptr<AppAssetRoots>;
    }
}
#endif //_JS_MODULESLOADER_H_
