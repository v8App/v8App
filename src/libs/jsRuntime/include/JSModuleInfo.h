// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_MODULEINFO_H_
#define _JS_MODULEINFO_H_

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"

#include "V8Types.h"
#include "JSModuleAttributesInfo.h"

namespace v8App
{
    namespace JSRuntime
    {

        class JSModuleInfo
        {
        public:
            inline static const char *kModuleTypeJS = "js";
            inline static const char *kModuleTypeJSON = "json";
            inline static const char *kModuleTypeNative = "native";

            struct SnapshotData
            {
                JSModuleType m_Type{JSModuleType::kInvalid};
                std::string m_Path;
                std::string m_ModuleName;
                std::string m_Version;
                bool m_SaveModule{false};
                bool m_SavedJSON{false};
                size_t m_ModuleDataIndex{0};
                size_t m_JSONModuleDataIndex{0};
                JSModuleAttributesInfo m_AtrribInfo;
            };

            static std::string ModuleTypeToString(JSModuleType inType);
            static JSModuleType StringToModuleType(std::string inString);

        public:
            JSModuleInfo(JSContextSharedPtr inContext);
            ~JSModuleInfo();

            void SetType(JSModuleType inType);
            JSModuleType GetType() const;
            
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

            void SetAttributesInfo(JSModuleAttributesInfo &inInfo);
            const JSModuleAttributesInfo &GetAttributesInfo() const;

            /**
             * Creates the snapshot data record for serialization
             */
            SnapshotData CreateSnapshotData(V8SnapshotCreatorSharedPtr inCreator);
            /**
             * Restores the v8 module from the snapshot data
             */
            void RestoreV8Module(const SnapshotData &inSnapData);

        protected:
            /**
             * The type of this module
             */
            JSModuleType m_Type{JSModuleType::kInvalid};

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

            // holds the unbound module script to generate code cache
            V8GUnboundModScript m_UnboundScript;

            // attributes info the module was loaded with
            JSModuleAttributesInfo m_AttributesInfo;

            // The context this module info is bound to
            JSContextSharedPtr m_Context;

        private:
            JSModuleInfo(const JSModuleInfo &) = delete;
            JSModuleInfo &operator=(JSModuleInfo &) = delete;
        };
    }
    template <>
    struct Serialization::TypeSerializer<JSRuntime::JSModuleInfo::SnapshotData>
    {
        static bool SerializeRead(Serialization::ReadBuffer &inBuffer, JSRuntime::JSModuleInfo::SnapshotData &inValue);
        static bool SerializeWrite(Serialization::WriteBuffer &inBuffer, const JSRuntime::JSModuleInfo::SnapshotData &inValue);
    };

    template <>
    struct Serialization::TypeSerializer<JSRuntime::JSModuleType>
    {
        static bool SerializeRead(Serialization::ReadBuffer &inBuffer, JSRuntime::JSModuleType &inValue);
        static bool SerializeWrite(Serialization::WriteBuffer &inBuffer, const JSRuntime::JSModuleType &inValue);
    };
}
#endif //_JS_MODULEINFO_H_
