// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_MODULES_H_
#define _JS_MODULES_H_

#include <unordered_map>
#include <filesystem>
#include <map>

#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSContext;

        using WeakJSModulesPtr = std::weak_ptr<class JSModules>;
        using SharedJSModulesPtr = std::shared_ptr<JSModules>;

        namespace internal
        {
            struct ModuleCallbackData
            {
                ModuleCallbackData(JSContext *inContext, v8::Local<v8::String> inReferrer, v8::Local<v8::String> inSpecifier,
                                   v8::Local<v8::Promise::Resolver> inResolver, v8::Local<v8::Value> inNamespace);

                JSContext *m_Context;
                v8::Global<v8::String> m_Referrer;
                v8::Global<v8::String> m_Specifier;
                v8::Global<v8::Promise::Resolver> m_Resolver;
                v8::Global<v8::Value> m_Namespace;
            };
        }

        class JSModules
        {
        public:
            enum ImportType
            {
                File,
                Module,
                Url
            };

            JSModules(JSContext *inContext, v8::Isolate *inIsolate) : m_Context(inContext), m_ModuleToSpecifierMap(10, ModuleHash(inIsolate)) {}
            ~JSModules();

            v8::Isolate *GetIsolate() const;

            v8::MaybeLocal<v8::Module> LoadModule(std::string inModulePath);
            bool InstantiateModule(v8::Local<v8::Module> inModule);
            bool RunModule(v8::Local<v8::Module> inModule);

            bool AddModule(const v8::Global<v8::Module> &inModule, std::string inReferrer);
            v8::MaybeLocal<v8::Module> GetModuleBySpecifier(std::string inSpecifier);
            std::string GetSpecifierByModule(const v8::Global<v8::Module> &inModule);

            void ResetModules();

            static void SetupModulesCallbacks(v8::Isolate *inIsolate);

            //Module Path related
            static bool AddModuleRootPath(std::string inModuleNmae, std::string inPath);
            static void RemoveModuleRootPath(std::string inModuleName);
            static std::filesystem::path FindModuleRootPath(std::string inModule);
            static void RemoveAllRootPaths();

        protected:
            static bool ParseImport(std::string& inImport, std::string &outModuleName, ImportType& outType);
            static bool DidModulePathEscapeRoot(std::filesystem::path inRootPath, std::filesystem::path inScriptPath);

            static v8::MaybeLocal<v8::Promise> ImportModuleDynamically(v8::Local<v8::Context> inContext, v8::Local<v8::ScriptOrModule> inReferrer, v8::Local<v8::String> inSpecifier);
            static void InitializeImportMetaObject(v8::Local<v8::Context> inContext, v8::Local<v8::Module> inModule, v8::Local<v8::Object> inMeta);
            static void ResolvePromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo);
            static void RejectPromiseCallback(const v8::FunctionCallbackInfo<v8::Value> &inInfo);
            static v8::MaybeLocal<v8::Module> ResolveModuleCallback(v8::Local<v8::Context> inContext, v8::Local<v8::String> inSpecifier, v8::Local<v8::Module> inReferrer);
            static void ImportModuleDynamicallyMicroTask(void *inData);
            static v8::MaybeLocal<v8::Module> LoadModuleTree(JSContext *inContext, const std::string &inImportPath, std::filesystem::path inModuleRoot);

        private:
            JSContext *m_Context;

            class ModuleHash
            {
            public:
                explicit ModuleHash(v8::Isolate *inIsolate) : m_Isolate(inIsolate) {}
                size_t operator()(const v8::Global<v8::Module> &inModule) const
                {
                    return inModule.Get(m_Isolate)->GetIdentityHash();
                }

            private:
                v8::Isolate *m_Isolate;
            };

            std::unordered_map<std::string, v8::Global<v8::Module>> m_SpecifierToModuleMap;
            std::unordered_map<v8::Global<v8::Module>, std::string, ModuleHash> m_ModuleToSpecifierMap;

            static std::map<std::string, std::filesystem::path> s_ModuleRoots;
            //module imports can't escape out of this directory
            static std::filesystem::path s_RootModulePath;

            JSModules(const JSModules &) = delete;
            JSModules &operator=(const JSModules &) = delete;
        };
    }
}
#endif
