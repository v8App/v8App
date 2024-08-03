// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"

#include "Serialization/ReadBuffer.h"
#include "Serialization/TypeSerializer.h"

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8CppObjInfo.h"
#include "JSContext.h"
#include "JSRuntime.h"
#include "JSApp.h"
#include "V8AppSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool V8AppSnapshotProvider::LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath)
        {
            if (m_Loaded == false)
            {
                if (LoadDataFile(inApp, inSnapshotPath))
                {
                    m_Loaded = true;
                }
            }

            return m_Loaded;
        }

        bool V8AppSnapshotProvider::LoadDataFile(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath)
        {
            if (inSnapshotPath.empty() == false)
            {
                m_SnapshotPath = inSnapshotPath;
            }
            if (m_SnapshotPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, "A path needs to be specified at construction or passed to LoadSnapshotData"}};
                LOG_ERROR(msg);
                return false;
            }
            std::filesystem::path absPath = inApp->GetAppRoot()->MakeAbsolutePathToAppRoot(m_SnapshotPath);
            if (absPath.empty())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Specified snapshot path may have escaped the app root. File:{}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            if (std::filesystem::exists(absPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            m_SnapshotPath = absPath;

            std::ifstream snapData(absPath, std::ios_base::binary | std::ios_base::ate);
            if (snapData.is_open() == false || snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            int dataLength = snapData.tellg();
            snapData.seekg(0, std::ios::beg);
            std::unique_ptr<char> buf = std::unique_ptr<char>(new char[dataLength]);
            snapData.read(buf.get(), dataLength);
            if (snapData.fail())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to read the snapshot file {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }
            m_Snapshots.insert(std::make_pair("v8", V8StartupData{buf.release(), dataLength}));
            return true;
        }

        const intptr_t *V8AppSnapshotProvider::GetExternalReferences()
        {
            return CppBridge::CallbackRegistry::GetReferences().data();
        }

        void V8AppSnapshotProvider::DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload)
        {
            Serialization::ReadBuffer rBuffer(inPayload.data, inPayload.raw_size);

            if (inIndex == (int)V8CppObjDataIntField::ObjInfo)
            {
                std::string typeName;
                rBuffer >> typeName;
                if (rBuffer.HasErrored())
                {
                    // Should throw an error here
                    return;
                }
                CppBridge::V8CppObjInfo *typeInfo = CppBridge::CallbackRegistry::GetNativeObjectInfoFromTypeName(typeName);
                if (typeInfo == nullptr)
                {
                    // should throw an error here
                    return;
                }
                inHolder->SetAlignedPointerInInternalField((int)V8CppObjDataIntField::ObjInfo, typeInfo);
                return;
            }
            if (inIndex == (int)V8CppObjDataIntField::ObjInstance)
            {
                CppBridge::V8CppObjInfo *objInfo = static_cast<CppBridge::V8CppObjInfo *>(inHolder->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInfo));
                if (objInfo == nullptr)
                {
                    // should probably throw an error
                    return;
                }
                CppBridge::V8CppObjectBase *obj = objInfo->m_Deserializer(inHolder->GetIsolate(), inHolder, rBuffer);
                if (obj == nullptr)
                {
                    // should probablt throw an error
                    return;
                }
                inHolder->SetAlignedPointerInInternalField((int)V8CppObjDataIntField::ObjInstance, obj);
            }
        }

        void V8AppSnapshotProvider::DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload)
        {
            // todo deserialize the context
        }
    }
}