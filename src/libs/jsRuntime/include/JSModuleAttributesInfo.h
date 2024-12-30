// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_MODULE_ATTRIBUTE_INFO_H_
#define _JS_MODULE_ATTRIBUTE_INFO_H_

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Serialization/TypeSerializer.h"
#include "Utils/VersionString.h"

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        struct JSModuleAttributesInfo
        {
            inline static const char *kExtJS = ".js";
            inline static const char *kExtModuleJS = ".mjs";
            inline static const char *kExtJSON = ".json";
            inline static const char *kExtNative = ".vbin";

            inline static const char *kAttribKeyModule = "module";
            inline static const char *kAttribKeyVersion = "version";
            inline static const char *kAttribKeyType = "type";

            /**
             * The type of module
             */
            JSModuleType m_Type{JSModuleType::kNoAttribute};
            /**
             * The version of the module
             */
            Utils::VersionString m_Version;
            /**
             * The module name to pin the import path to
             */
            std::string m_Module;

            bool DoesExtensionMatchType(std::string inExt) const;
            bool GetModuleAttributesInfo(JSContextSharedPtr inContext, V8LFixedArray inAttributes);
        };
    }

    template <>
    struct Serialization::TypeSerializer<JSRuntime::JSModuleAttributesInfo>
    {
        static bool SerializeRead(Serialization::ReadBuffer &inBuffer, JSRuntime::JSModuleAttributesInfo &inValue);
        static bool SerializeWrite(Serialization::WriteBuffer &inBuffer, const JSRuntime::JSModuleAttributesInfo &inValue);
    };
}
#endif //_JS_MODULE_ATTRIBUTE_INFO_H_
