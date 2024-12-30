// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSModuleAttributesInfo.h"

#include "Logging/LogMacros.h"
#include "Utils/Format.h"

#include "JSContext.h"
#include "JSUtilities.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool JSModuleAttributesInfo::DoesExtensionMatchType(std::string inExt) const
        {
            if (inExt == kExtJSON && m_Type == JSModuleType::kJSON)
            {
                return true;
            }
            if ((inExt == kExtJS || inExt == kExtModuleJS) && m_Type == JSModuleType::kJavascript)
            {
                return true;
            }
            if (inExt == kExtNative && m_Type == JSModuleType::kNative)
            {
                return true;
            }
            return false;
        }

        bool JSModuleAttributesInfo::GetModuleAttributesInfo(JSContextSharedPtr inContext, V8LFixedArray inAttributes)
        {
            V8Isolate *isolate = inContext->GetIsolate();
            m_Type = JSModuleType::kJavascript;
            V8LContext context = inContext->GetLocalContext();

            if (isolate == nullptr)
            {
                // TODO:: Log message
                m_Type = JSModuleType::kInvalid;
                return false;
            }
            int arrayLen = inAttributes->Length();
            // we take advantage of the fact that the attributes are pairs if source offset is added it's a triplet
            const int kEntrySize = arrayLen % 2 ? 3 : 2;
            bool invalidAttribute = false;

            for (int i = 0; i < arrayLen; i += kEntrySize)
            {
                V8LString v8Key = inAttributes->Get(context, i).As<V8String>();
                std::string key = JSUtilities::V8ToString(isolate, v8Key);
                // for some reason we can end up with a blank key
                if (key == "")
                {
                    continue;
                }
                if (key == kAttribKeyType || key == kAttribKeyModule || key == kAttribKeyVersion)
                {
                    V8LString v8Value = inAttributes->Get(context, i + 1).As<V8String>();
                    std::string value = JSUtilities::V8ToString(isolate, v8Value);
                    if (key == kAttribKeyType)
                    {
                        if (value == JSModuleInfo::kModuleTypeJSON)
                        {
                            m_Type = JSModuleType::kJSON;
                        }
                        else if (value == JSModuleInfo::kModuleTypeJS)
                        {
                            m_Type = JSModuleType::kJavascript;
                        }
                        else if (value == JSModuleInfo::kModuleTypeNative)
                        {
                            m_Type = JSModuleType::kNative;
                        }
                        else
                        {
                            LOG_WARN(Utils::format("Unknown {} attribute: {}", kAttribKeyType, value));

                            m_Type = JSModuleType::kInvalid;
                            invalidAttribute = true;
                        }
                    }
                    else if (key == kAttribKeyModule)
                    {
                        m_Module = value;
                    }
                    else if (key == kAttribKeyVersion)
                    {
                        m_Version.SetVersionString(value);
                        if(m_Version.IsVersionString() == false)
                        {
                            LOG_WARN(Utils::format("Invalid {} attribute: {}", kAttribKeyVersion, value));

                            m_Type = JSModuleType::kInvalid;
                            invalidAttribute = true;
                        }
                    }
                }
                else
                {
                    //We just wanr on uknown keys
                    Log::LogMessage message = {
                        {Log::MsgKey::Msg, Utils::format("Unknown attribute: {}", key)},
                    };
                    LOG_WARN(message);
                }
            }

            return invalidAttribute == false;
        }
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSModuleAttributesInfo>::SerializeRead(Serialization::ReadBuffer &inBuffer, v8App::JSRuntime::JSModuleAttributesInfo &inValue)
    {
        inBuffer >> inValue.m_Type;
        inBuffer >> inValue.m_Module;
        inBuffer >> inValue.m_Version;
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSModuleAttributesInfo>::SerializeWrite(Serialization::WriteBuffer &inBuffer, const v8App::JSRuntime::JSModuleAttributesInfo &inValue)
    {
        inBuffer << inValue.m_Type;
        inBuffer << inValue.m_Module;
        inBuffer << inValue.m_Version;
        return inBuffer.HasErrored() == false;
    }
} // namespace v8App
