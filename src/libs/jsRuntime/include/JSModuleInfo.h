// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_MODULEINFO_H_
#define _JS_MODULEINFO_H_

#include <unordered_map>
#include <filesystem>
#include <map>

#include "Utils/VersionString.h"
#include "Utils/Format.h"

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSModuleInfo
        {
        public:
            enum class ModuleType
            {
                kInvalid,
                kJavascript,
                kJSON,
                kNative
            };

            static std::string ModuleTypeToString(ModuleType inType)
            {
#define STRINGIFY(inType) (#inType)
#define MI_MATCHER(inType, moduleType) \
    if (inType == moduleType)       \
    return std::string(STRINGIFY(moduleType)).replace(0, 13, "")
                MI_MATCHER(inType, ModuleType::kInvalid);
                MI_MATCHER(inType, ModuleType::kJavascript);
                MI_MATCHER(inType, ModuleType::kJSON);
                MI_MATCHER(inType, ModuleType::kNative);
#undef MI_MATCHER
#undef STRINGIFY
                return Utils::format("Unknown ModuleType enum {}, perhaps need to delcare it's macro in ModuleTypeToString", (int)inType);
            }

            struct AttributesInfo
            {
                ModuleType m_Type = ModuleType::kInvalid;
                std::string m_TypeString;
                std::string m_Module;

                bool DoesExtensionMatchType(std::string inExt) const
                {
                    if(inExt == ".json" && m_Type == ModuleType::kJSON)
                    {
                        return true;
                    }
                    if((inExt == ".js" || inExt == ".mjs") && m_Type == ModuleType::kJavascript)
                    {
                        return true;
                    }
                    //TODO: fogure out the native extension and add it
                    return false;
                }
            };

        public:
            JSModuleInfo(JSContextSharedPtr inContext);
            ~JSModuleInfo();

            void SetPath(std::filesystem::path inPath);
            std::filesystem::path GetModulePath() const;

            void SetName(std::string inName);
            std::string GetName() const;
            
            void SetVersion(std::string inVersion);
            Utils::VersionString GetVersion() const;
            
            void SetV8Module(V8LModule &inModule);
            V8LModule GetLocalModule();
            
            void SetV8JSON(V8LValue &inValue);
            V8LValue GetLocalJSON();

            void SetUnboundScript(V8LUnboundModScript &inScript);
            V8LUnboundModScript GetUnboundScript();
            void ClearUnboundScript();

            void SetAttributesInfo(AttributesInfo &inInfo);
            const AttributesInfo &GetAttributesInfo() const;

        protected:
            // the actual path to the module file
            std::filesystem::path m_Path;
            // the module that the file is under
            std::string m_ModuleName;
            // modules version
            Utils::VersionString m_Version;
            // the loaded module
            V8GModule m_Module;
            // the parsed json if it's a json module
            V8GValue m_JSON;
            // attributes info the module was loaded with
            AttributesInfo m_AttributesInfo;
            //holds the unbound module script to generate code cache
            V8GUnboundModScript m_UnboundScript;
            
            JSContextSharedPtr m_Context;

        private:
            JSModuleInfo(const JSModuleInfo &) = delete;
            JSModuleInfo &operator=(JSModuleInfo &) = delete;
        };

        using JSModuleInfoSharedPtr = std::shared_ptr<JSModuleInfo>;
        using JSModuleInfoWeakPre = std::weak_ptr<JSModuleInfo>;
    }
}
#endif //_JS_MODULEINFO_H_
