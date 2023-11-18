// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_MODULES_H_
#define _JS_CONTEXT_MODULES_H_

#include <unordered_map>
#include <filesystem>
#include <map>

#include "JSModuleInfo.h"
#include "Utils/VersionString.h"

#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        using JSContextSharedPtr = std::shared_ptr<JSContext>;
        using JSContextModulesWeakPtr = std::weak_ptr<class JSContextModules>;
        using JSContextModulesSharedPtr = std::shared_ptr<JSContextModules>;

        class JSContextModules
        {
        public:

            JSContextModules(JSContextSharedPtr inContext);
            ~JSContextModules();

            v8::Isolate *GetIsolate() const;

            /**
             * LoadEntryPoint takes a path that should be rooted under the app root and can be absolute or relative.
             * Used to load the main entry point script.
             */
            v8::MaybeLocal<v8::Module> LoadEntryPoint(std::filesystem::path inMainPath);
            bool InstantiateModule(v8::Local<v8::Module> inModule);
            bool RunModule(v8::Local<v8::Module> inModule);

            /**
             * Adds a module to the map tracking that it's been loaded
             */
            bool AddModule(const JSModuleInfoSharedPtr &inModule, std::string inFileName, JSModuleInfo::ModuleType inModuleType);
            /**
             * Adds parsed JSON to the map tracking that it's been loaded
             */
            bool AddJSONModule(const JSModuleInfoSharedPtr &inModule, v8::Global<v8::Value> &inParsedJSON, std::string inFileName,  JSModuleInfo::ModuleType inModuleType);
            /**
             * Finds the module by the specified module specifier ie name
             */
            JSModuleInfoSharedPtr GetModuleBySpecifier(std::string inSpecifier);
            /**
             * Gets the module specifier by the module
             */
            std::string GetSpecifierByModule(const JSModuleInfoSharedPtr &inModule);
            /**
             * Gets the parsed json by the module
             */
            v8::Local<v8::Value> GetJSONByModule(const JSModuleInfoSharedPtr &inModule);
            /**
             * Resets all the maps
             */
            void ResetModules();

            /**
             * Sets up the callbacks for v8 to do imports
             */
            static void SetupModulesCallbacks(v8::Isolate *inIsolate);

        protected:

            /**
             * Parses the import assertion and returns the info asserted
            */
            JSModuleInfo::AssertionInfo GetModuleAssertionInfo(v8::Local<v8::Context> inContext, v8::Local<v8::FixedArray> inAssertions, bool inHasPostions);
            bool BuildModulePath(const JSModuleInfo::AssertionInfo& inAssertionInfo, std::filesystem::path& inImportPath);

            /**
             * Callback for V8 to dynamically load imports
             */
            static v8::MaybeLocal<v8::Promise> HostImportModuleDynamically(v8::Local<v8::Context> inContext,
                                                                           v8::Local<v8::Data> inDefinedOptions,
                                                                           v8::Local<v8::Value> inResourceName,
                                                                           v8::Local<v8::String> inSpecifier,
                                                                           v8::Local<v8::FixedArray> import_assertions);
            /**
             * A Microtask run by V8 that handles the actual importing
             */
            static void ImportModuleDynamicallyMicroTask(void *inData);
            /**
             * Callabck for V8 to initialized the import meta object
             */
            static void InitializeImportMetaObject(v8::Local<v8::Context> inContext, v8::Local<v8::Module> inModule, v8::Local<v8::Object> inMeta);
            /**
             * Resolve the modules promise
             */
            static void ResolvePromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo);
            /**
             * Rejects the modules promise
             */
            static void RejectPromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo);
            /**
             *
             */
            static v8::MaybeLocal<v8::Module> ResolveModuleCallback(v8::Local<v8::Context> inContext, v8::Local<v8::String> inSpecifier, v8::Local<v8::FixedArray> inAssertions, v8::Local<v8::Module> inReferrer);
            /**
             * Handles loading additional imports from the imported module
             */
            static v8::MaybeLocal<v8::Module> LoadModuleTree(JSContextSharedPtr inContext, const std::filesystem::path &inImportPath, std::filesystem::path inModuleRoot, const  JSModuleInfo::AssertionInfo& inAssertionInfo);

        private:
            JSContextSharedPtr m_Context;

            /**
             * Class used for comparing modules when searching the maps
             */
            class ModuleHash
            {
            public:
                explicit ModuleHash(JSContextSharedPtr inContext) : m_Context(inContext) {}
                size_t operator()(const JSModuleInfoSharedPtr &inModule) const;
                JSContextSharedPtr m_Context;
            };

            std::map<std::pair<std::string, JSModuleInfo::ModuleType>, JSModuleInfoSharedPtr> m_ModuleMap;
            std::unordered_map<JSModuleInfoSharedPtr, std::string, ModuleHash> m_ModuleToSpecifierMap;
            std::unordered_map<JSModuleInfoSharedPtr, v8::Global<v8::Value>, ModuleHash> m_JSONModuleToParsedMap;

            JSContextModules(const JSContextModules &) = delete;
            JSContextModules &operator=(const JSContextModules &) = delete;
        };
    }
}
#endif //_JS_CONTEXT_MODULES_H_
