// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_MODULES_H_
#define _JS_CONTEXT_MODULES_H_

#include <unordered_map>
#include <filesystem>
#include <map>

#include "JSModuleInfo.h"
#include "V8Types.h"
#include "Utils/VersionString.h"

#include "v8.h"

/**
 * module paths
 * If no extension is specified we see if a file with .js or .mjs exists. If the path points
 * at a driectory then we look for a file index.js or index.mjs.
 *
 * path parts
 * approot/[js|modules|resources]/<path to file>
 * modules path os also this where version can be optional
 * approot/modules/<module name>/<version>/<path to file>
 *
 * Paths example skipping any extension added based on importRoot. that are not absolute are always relative to the import Path.
 * ./test - importRoot/test
 * test - check for module name, importRoot/test
 * /approot/modules/test
 * /approot/modules/test/1.2.3
 * modules/test
 * appRoot/js/test
 * js/test
 * appRoot/resources/test
 * resources/test
 *
 * If Path starts with % then possible token
 * If path starts with ./ then relative to the importRoot
 * If no ./ then check if abs path
 *  if not abs check first part to see if it's js, modules, resources if so add app root
 *
 *
 */
namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        class JSContextModules
        {
        public:
            JSContextModules(JSContextSharedPtr inContext);
            ~JSContextModules();

            V8Isolate *GetIsolate() const;

            /**
             * LoadEntryPoint takes a path that should be rooted under the app root and can be absolute or relative.
             */
            V8MaybeLocalModule LoadModule(std::filesystem::path inModulePath);
            bool InstantiateModule(JSModuleInfoSharedPtr inModule);
            bool RunModule(JSModuleInfoSharedPtr inModule);

            /**
             * Finds the module by the specified module specifier ie name
             */
            JSModuleInfoSharedPtr GetModuleBySpecifier(std::string inSpecifier);
            /**
             * Gets the module specifier by the module
             */
            std::string GetSpecifierByModule(V8LocalModule inModule);
            /**
             * Gets the parsed json by the module
             */
            V8LocalValue GetJSONByModule(V8LocalModule inModule);
            /**
             * Resets all the maps
             */
            void ResetModules();

            /**
             * Sets up the callbacks for v8 to do imports
             */
            static void SetupModulesCallbacks(V8Isolate *inIsolate);

        protected:
            // Generate the code cache for the module and sets the CodeCache with it.
            bool GenerateCodeCache(std::filesystem::path inImportPath, V8ScriptSource *inSource, V8Isolate *inIsolate, V8LocalContext inContext);

            /**
             * Adds a module to the map tracking that it's been loaded
             */
            bool AddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType);
            JSModuleInfoSharedPtr GetModuleInfoByModule(V8LocalModule inModule, JSModuleInfo::ModuleType inType = JSModuleInfo::ModuleType::kInvalid);

            /**
             * Parses the import assertion and returns the info asserted
             */
            JSModuleInfo::AssertionInfo GetModuleAssertionInfo(JSContextSharedPtr inContext, V8LocalFixedArray inAssertions, bool inHasPostions);

            JSModuleInfoSharedPtr BuildModuleInfo(JSModuleInfo::AssertionInfo &inAssertionInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath);

            /**
             * Callback for V8 to dynamically load imports
             */
            static V8MaybeLocalPromise HostImportModuleDynamically(V8LocalContext inContext,
                                                                   V8LocalData inDefinedOptions,
                                                                   V8LocalValue inResourceName,
                                                                   V8LocalString inSpecifier,
                                                                   V8LocalFixedArray import_assertions);

            /**
             * A Microtask run by V8 that handles the actual importing
             */
            static void ImportModuleDynamicallyMicroTask(void *inData);

            /**
             * Callabck for V8 to initialized the import meta object
             */
            static void InitializeImportMetaObject(V8LocalContext inContext, V8LocalModule inModule, V8LocalObject inMeta);

            /**
             * Callback to handle parsing JSON
             */
            static V8MaybeLocalValue JSONEvalutionSteps(V8LocalContext inContext, V8LocalModule inModule);

            /**
             * Resolve the modules promise
             */
            static void ResolvePromiseCallback(const V8FuncCallInfoValue &inInfo);

            /**
             * Rejects the modules promise
             */
            static void RejectPromiseCallback(const V8FuncCallInfoValue &inInfo);

            /**
             *
             */
            static V8MaybeLocalModule ResolveModuleCallback(V8LocalContext inContext, V8LocalString inSpecifier, V8LocalFixedArray inAssertions, V8LocalModule inReferrer);

            /**
             * Handles loading additional imports from the imported module
             */
            static V8MaybeLocalModule LoadModuleTree(JSContextSharedPtr inContext, std::filesystem::path inModuleRoot, const JSModuleInfoSharedPtr inModuleInfo);

        protected:
            JSContextSharedPtr m_Context;

            std::map<std::pair<std::string, JSModuleInfo::ModuleType>, JSModuleInfoSharedPtr> m_ModuleMap;

            JSContextModules(const JSContextModules &) = delete;
            JSContextModules &operator=(const JSContextModules &) = delete;
        };
    }
}
#endif //_JS_CONTEXT_MODULES_H_
