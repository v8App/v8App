// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"

#include "JSModuleInfo.h"
#include "JSContext.h"
namespace v8App
{
    namespace JSRuntime
    {

        std::string JSModuleInfo::ModuleTypeToString(JSModuleType inType)
        {
#define STRINGIFY(inType) (#inType)
#define MI_MATCHER(inType, moduleType) \
    if (inType == moduleType)          \
    return std::string(STRINGIFY(moduleType)).replace(0, 15, "")
            MI_MATCHER(inType, JSModuleType::kInvalid);
            MI_MATCHER(inType, JSModuleType::kJavascript);
            MI_MATCHER(inType, JSModuleType::kJSON);
            MI_MATCHER(inType, JSModuleType::kNative);
#undef MI_MATCHER
#undef STRINGIFY
            LOG_WARN(Utils::format("Unknown ModuleType enum {}, perhaps need to delcare it's macro in ModuleTypeToString", (int)inType));
            return "";
        }

        JSModuleType JSModuleInfo::StringToModuleType(std::string inString)
        {
            std::string typeString;
#define STRINGIFY(inType) (#inType)
#define MI_MATCHER(inType, moduleType)                                  \
    typeString = std::string(STRINGIFY(moduleType)).replace(0, 15, ""); \
    if (inType == typeString)                                           \
    return moduleType
            MI_MATCHER(inString, JSModuleType::kInvalid);
            MI_MATCHER(inString, JSModuleType::kJavascript);
            MI_MATCHER(inString, JSModuleType::kJSON);
            MI_MATCHER(inString, JSModuleType::kNative);
#undef MI_MATCHER
#undef STRINGIFY
            LOG_ERROR(Utils::format("Unknown ModuleType string {}, perhaps need to delcare it's macro in ModuleTypeToString", inString));
            return JSModuleType::kInvalid;
        }

        JSModuleInfo::JSModuleInfo(JSContextSharedPtr inContext) : m_Context(inContext)
        {
            CHECK_NE(inContext, nullptr);
        }

        JSModuleInfo::~JSModuleInfo()
        {
            m_Module.Reset();
            m_JSON.Reset();
            m_UnboundScript.Reset();
            m_Context.reset();
        }

        void JSModuleInfo::SetType(JSModuleType inType)
        {
            m_Type = inType;
        }
        JSModuleType JSModuleInfo::GetType() const
        {
            return m_Type;
        }

        void JSModuleInfo::SetPath(std::filesystem::path inPath)
        {
            m_Path = inPath;
        }

        std::filesystem::path JSModuleInfo::GetModulePath() const
        {
            return m_Path;
        }

        void JSModuleInfo::SetName(std::string inName)
        {
            m_ModuleName = inName;
        }

        std::string JSModuleInfo::GetName() const
        {
            return m_ModuleName;
        }

        void JSModuleInfo::SetVersion(std::string inVersion)
        {
            m_Version.SetVersionString(inVersion);
        }

        Utils::VersionString JSModuleInfo::GetVersion() const
        {
            return m_Version;
        }

        void JSModuleInfo::SetV8Module(V8LModule &inModule)
        {
            m_Module.Reset(m_Context->GetIsolate(), inModule);
        }

        V8LModule JSModuleInfo::GetLocalModule()
        {
            return m_Module.Get(m_Context->GetIsolate());
        }

        void JSModuleInfo::SetV8JSON(V8LValue &inValue)
        {
            m_JSON.Reset(m_Context->GetIsolate(), inValue);
        }

        V8LValue JSModuleInfo::GetLocalJSON()
        {
            return m_JSON.Get(m_Context->GetIsolate());
        }

        void JSModuleInfo::SetUnboundScript(V8LUnboundModScript &inScript)
        {
            m_UnboundScript.Reset(m_Context->GetIsolate(), inScript);
        }

        V8LUnboundModScript JSModuleInfo::GetUnboundScript()
        {
            return m_UnboundScript.Get(m_Context->GetIsolate());
        }
        void JSModuleInfo::ClearUnboundScript()
        {
            m_UnboundScript.Reset();
        }

        void JSModuleInfo::SetAttributesInfo(JSModuleAttributesInfo &inInfo)
        {
            m_AttributesInfo = inInfo;
        }

        const JSModuleAttributesInfo &JSModuleInfo::GetAttributesInfo() const
        {
            return m_AttributesInfo;
        }

        JSModuleInfo::SnapshotData JSModuleInfo::CreateSnapshotData(V8SnapshotCreatorSharedPtr inCreator)
        {
            V8Isolate *isolate = m_Context->GetIsolate();
            V8HandleScope hScope(isolate);
            V8LContext context = m_Context->GetLocalContext();

            SnapshotData data;

            data.m_Type = m_Type;
            data.m_Path = m_Path;
            data.m_Version = m_Version.GetVersionString();;
            data.m_ModuleName = m_ModuleName;
            m_Version.SetVersionString(data.m_Version);
            data.m_AtrribInfo = m_AttributesInfo;

            if (m_Module.IsEmpty() == false)
            {
                data.m_ModuleDataIndex = inCreator->AddData(context, m_Module.Get(isolate));
                // Have to release the global for the snapshot
                m_Module.Reset();
                data.m_SaveModule = true;
            }
            if (m_JSON.IsEmpty() == false)
            {
                data.m_JSONModuleDataIndex = inCreator->AddData(context, m_JSON.Get(isolate));
                m_JSON.Reset();
                data.m_SavedJSON = true;
            }
            if (m_UnboundScript.IsEmpty() == false)
            {
                // Doesn't seem to be a way to recover this since the cast back from the Maybe doesn't exist
                // m_CreatorScriptIndex = inCreator->AddData(context, m_UnboundScript.Get(isolate));
                m_UnboundScript.Reset();
                // m_SavedScript = true;
            }
            return data;
        }

        void JSModuleInfo::RestoreV8Module(const SnapshotData &inSnapData)
        {
            V8Isolate *isolate = m_Context->GetIsolate();
            V8IsolateScope iScope(isolate);
            V8HandleScope hScope(isolate);

            V8LContext context = m_Context->GetLocalContext();

            if (inSnapData.m_SaveModule)
            {
                m_Module.Reset(isolate, context->GetDataFromSnapshotOnce<V8Module>(inSnapData.m_ModuleDataIndex).ToLocalChecked());
            }
            if (inSnapData.m_SavedJSON)
            {
                m_JSON.Reset(isolate, context->GetDataFromSnapshotOnce<V8Value>(inSnapData.m_JSONModuleDataIndex).ToLocalChecked());
            }
            // if (m_SavedScript)
            //{
            //  Seems we can't convert it back from the MaybeLocal so we might need to do this another way
            //  m_UnboundScript.Reset(isolate, context->GetDataFromSnapshotOnce<V8UnboundModScript>(m_CreatorScriptIndex).ToLocalChecked());
            //}
        }
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSModuleInfo::SnapshotData>::SerializeRead(Serialization::ReadBuffer &inBuffer, v8App::JSRuntime::JSModuleInfo::SnapshotData &inValue)
    {
        inBuffer >> inValue.m_Type;
        inBuffer >> inValue.m_Path;
        inBuffer >> inValue.m_ModuleName;
        inBuffer >> inValue.m_Version;
        inBuffer >> inValue.m_SaveModule;
        if (inValue.m_SaveModule)
        {
            inBuffer >> inValue.m_ModuleDataIndex;
        }
        inBuffer >> inValue.m_SavedJSON;
        if (inValue.m_SavedJSON)
        {
            inBuffer >> inValue.m_JSONModuleDataIndex;
        }
        inBuffer >> inValue.m_AtrribInfo;
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSModuleInfo::SnapshotData>::SerializeWrite(Serialization::WriteBuffer &inBuffer, const v8App::JSRuntime::JSModuleInfo::SnapshotData &inValue)
    {
        inBuffer << inValue.m_Type;
        inBuffer << inValue.m_Path;
        inBuffer << inValue.m_ModuleName;
        inBuffer << inValue.m_Version;
        inBuffer << inValue.m_SaveModule;
        if (inValue.m_SaveModule)
        {
            inBuffer << inValue.m_ModuleDataIndex;
        }
        inBuffer << inValue.m_SavedJSON;
        if (inValue.m_SavedJSON)
        {
            inBuffer << inValue.m_JSONModuleDataIndex;
        }
        inBuffer << inValue.m_AtrribInfo;
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSModuleType>::SerializeRead(Serialization::ReadBuffer &inBuffer, v8App::JSRuntime::JSModuleType &inValue)
    {
        size_t modType;
        inBuffer >> modType;
        if (modType < 0 || modType >= (int)JSRuntime::JSModuleType::kMaxModType)
        {
            inBuffer.SetError();
            return false;
        }
        inValue = (JSRuntime::JSModuleType)modType;
        return inBuffer.HasErrored() == false;
    }

    bool Serialization::TypeSerializer<v8App::JSRuntime::JSModuleType>::SerializeWrite(Serialization::WriteBuffer &inBuffer, const v8App::JSRuntime::JSModuleType &inValue)
    {
        inBuffer << (size_t)inValue;
        return inBuffer.HasErrored() == false;
    }
}