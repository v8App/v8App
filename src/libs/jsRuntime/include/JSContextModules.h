// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
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

#include "v8/v8.h"

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
            JSModuleInfoSharedPtr LoadModule(std::filesystem::path inModulePath);
            bool InstantiateModule(JSModuleInfoSharedPtr inModule);
            V8LValue RunModule(JSModuleInfoSharedPtr inModule);

            /**
             * Finds the module by the specified module specifier ie name
             */
            JSModuleInfoSharedPtr GetModuleBySpecifier(std::string inSpecifier);
            /**
             * Gets the module specifier by the module
             */
            std::string GetSpecifierByModule(V8LModule inModule);
            /**
             * Gets the parsed json by the module
             */
            V8LValue GetJSONByModule(V8LModule inModule);
            /**
             * Resets all the maps
             */
            void ResetModules();

            /**
             * Sets up the callbacks for v8 to do imports
             */
            static void SetupModulesCallbacks(V8Isolate *inIsolate);

            bool 

            /**
             * Genrates the code cache for the modules that have been instantiated
             * and do not already have code cache data. NOTE: If modules are in a state 
             * of Evaluating, Evaluated or Errored then their code cache won't be generated.
            */
            void GenerateCodeCache();

        protected:
            /**
             * Adds a module to the map tracking that it's been loaded
             */
            bool AddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType);
            JSModuleInfoSharedPtr GetModuleInfoByModule(V8LModule inModule, JSModuleInfo::ModuleType inType = JSModuleInfo::ModuleType::kInvalid);

            /**
             * Parses the import Attributes and returns the info attributed
             */
            JSModuleInfo::AttributesInfo GetModuleAttributesInfo(JSContextSharedPtr inContext, V8LFixedArray inAttributes);

            JSModuleInfoSharedPtr BuildModuleInfo(JSModuleInfo::AttributesInfo &inAttributesInfo, const std::filesystem::path &inImportPath, const std::filesystem::path &inCurrentModPath);

            /**
             * Callback for V8 to dynamically load imports
             */
            static V8MBLPromise HostImportModuleDynamically(V8LContext inContext,
                                                                   V8LData inDefinedOptions,
                                                                   V8LValue inResourceName,
                                                                   V8LString inSpecifier,
                                                                   V8LFixedArray import_attributes);

            /**
             * A Microtask run by V8 that handles the actual importing
             */
            static void ImportModuleDynamicallyMicroTask(void *inData);

            /**
             * Callabck for V8 to initialized the import meta object
             */
            static void InitializeImportMetaObject(V8LContext inContext, V8LModule inModule, V8LObject inMeta);

            /**
             * Callback to handle parsing JSON
             */
            static V8MBLValue JSONEvalutionSteps(V8LContext inContext, V8LModule inModule);

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
            static V8MBLModule ResolveModuleCallback(V8LContext inContext, V8LString inSpecifier, V8LFixedArray inAttributes, V8LModule inReferrer);

            /**
             * Handles loading additional imports from the imported module
             */
            static JSModuleInfoSharedPtr LoadModuleTree(JSContextSharedPtr inContext, const JSModuleInfoSharedPtr inModuleInfo);

            JSContextSharedPtr m_Context;

            std::map<std::pair<std::string, JSModuleInfo::ModuleType>, JSModuleInfoSharedPtr> m_ModuleMap;

            JSContextModules(const JSContextModules &) = delete;
            JSContextModules &operator=(const JSContextModules &) = delete;
        };
    }
}
#endif //_JS_CONTEXT_MODULES_H_
