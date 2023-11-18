// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_MODULEINFO_H_
#define _JS_MODULEINFO_H_

#include <unordered_map>
#include <filesystem>
#include <map>

#include "Utils/VersionString.h"

#include "v8.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSModuleInfo
        {
        public:
            enum ModuleType
            {
                kInvalid,
                kJavascript,
                kJSON,
                kNative
            };

            struct AssertionInfo
            {
                ModuleType m_Type;
                std::string m_TypeString;
                std::string m_Module;
                Utils::VersionString m_Version;
            };

        public:
            JSModuleInfo() {}
            ~JSModuleInfo()
            {
                m_Module.Reset();
            }

            std::filesystem::path GetModulePath() const { return m_Path; }
            std::string GetName() const { return m_ModuleName; }
            Utils::VersionString GetVersion() const { return m_Version; }
            v8::Local<v8::Module> GetV8Module(v8::Isolate *isolate) { return m_Module.Get(isolate); }

            void SetAssertionInfo(AssertionInfo &inInfo) { m_AssertionInfo = inInfo; }
            const AssertionInfo &GetAssertionInfo() const { return m_AssertionInfo; }

        protected:
            void SetName(std::string &inName) { m_ModuleName = inName; }
            void SetVersion(std::string &inVersion) { m_Version = inVersion; }
            void SetPath(std::filesystem::path &inPath) { m_Path = inPath; }
            void SetV8Module(v8::Local<v8::Module> &inModule, v8::Isolate *inIsolate) { m_Module.Reset(inIsolate, inModule); }

            // the actual path to the module file
            std::filesystem::path m_Path;
            // the module that the file is under
            std::string m_ModuleName;
            // modules version
            Utils::VersionString m_Version;
            // the loaded module
            v8::Global<v8::Module> m_Module;
            // Assertion info the module was loaded with
            AssertionInfo m_AssertionInfo;

        private:
            JSModuleInfo(const JSModuleInfo &) = delete;
            JSModuleInfo &operator=(JSModuleInfo &) = delete;

            friend class JSContextModules;
        };

        using JSModuleInfoSharedPtr = std::shared_ptr<JSModuleInfo>;
        using JSModuleInfoWeakPre = std::weak_ptr<JSModuleInfo>;
    }
}
#endif //_JS_MODULEINFO_H_
