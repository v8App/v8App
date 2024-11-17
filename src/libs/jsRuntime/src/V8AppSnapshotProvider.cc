// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"

#include "Serialization/ReadBuffer.h"
#include "Serialization/TypeSerializer.h"
#include "Assets/BinaryAsset.h"

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8CppObjInfo.h"
#include "JSContext.h"
#include "JSRuntime.h"
#include "JSApp.h"
#include "V8AppSnapshotProvider.h"
#include "JSRuntimeVersion.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool V8AppSnapshotProvider::LoadSnapshotData(std::filesystem::path inSnapshotPath)
        {
            if (m_Loaded && m_SnapshotPath == inSnapshotPath)
            {
                return true;
            }

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

            if (std::filesystem::exists(m_SnapshotPath) == false)
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Passed snapshot path doesn't exist {}", m_SnapshotPath)}};
                LOG_ERROR(msg);
                return false;
            }

            m_Loaded = false;
            Assets::BinaryAsset m_AssetFile;
            m_AssetFile.SetPath(m_SnapshotPath);

            if (m_AssetFile.ReadAsset() == false)
            {
                LOG_ERROR(Utils::format("Failed to read the snapshot file {}", m_SnapshotPath));
                return false;
            }

            Serialization::ReadBuffer rBuffer(m_AssetFile.GetContent().data(), m_AssetFile.GetContent().size());

            uint32_t v8AppMagicNumber = 0;
            uint32_t major_ver = 0;
            uint32_t minor_ver = 0;
            uint32_t patch_ver = 0;
            uint32_t build_ver = 0;

            // just resue the var to make sure the first 4 bytes are 0
            rBuffer >> v8AppMagicNumber;
            if (v8AppMagicNumber != 0)
            {
                LOG_ERROR("Magic Number in file is not 0");
                return false;
            }
            // check the next 4 are our magic number
            // TODO: figure out our magic number
            rBuffer >> v8AppMagicNumber;
            if (v8AppMagicNumber != 0)
            {
                LOG_ERROR("Magic Number in file is not 0");
                return false;
            }

            rBuffer >> major_ver;
            rBuffer >> minor_ver;
            rBuffer >> patch_ver;
            rBuffer >> build_ver;

            // TODO add a table of some sort that has version comapatiblitly
            if (major_ver != JSRUNTIME_MAJOR_VERSION || minor_ver != JSRUNTIME_MINOR_VERSION || patch_ver != JSRUNTIME_PATCH_LEVEL)
            {
                LOG_ERROR(Utils::format("Files runtime version of {}.{].{} doesn't match apps version {}.{].{}}", major_ver, minor_ver, patch_ver, JSRUNTIME_MAJOR_VERSION, JSRUNTIME_MINOR_VERSION, JSRUNTIME_PATCH_LEVEL));
                return false;
            }
            std::string platformArch;
            rBuffer >> platformArch;
            // TODO: add platform check

            std::string appClassType;

            rBuffer >> appClassType;

            JSAppSharedPtr app = JSAppCreatorRegistry::CreateApp(appClassType);
            if (app == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to find JSApp type {} registered with the JSAppCreator", appClassType));
                return false;
            }
            m_SnapData = app->LoadSnapshotData(rBuffer);
            if (m_SnapData == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to load the snapshot data for JSAppType {}.", appClassType));
                return false;
            }

            return true;
        }

        const V8StartupData *V8AppSnapshotProvider::GetSnapshotData(size_t inIndex)
        {
            if (inIndex < 0 || m_SnapData->m_RuntimesSnapData.size() < inIndex)
            {
                return nullptr;
            }
            return &m_SnapData->m_RuntimesSnapData[inIndex]->m_StartupData;
        }

        size_t V8AppSnapshotProvider::GetIndexForRuntimeName(std::string inRuntimeName)
        {
            size_t runtimeIndex = m_SnapData->m_RuntimesSnapIndexes.GetIndexForName(inRuntimeName);
            if(runtimeIndex == m_SnapData->m_RuntimesSnapIndexes.GetMaxSupportedIndexes())
            {
                return 0;
            }
            return runtimeIndex;
        }

        const intptr_t *V8AppSnapshotProvider::GetExternalReferences()
        {
            return CppBridge::CallbackRegistry::GetReferences().data();
        }

        JSAppSharedPtr V8AppSnapshotProvider::CreateApp()
        {
            return JSAppSharedPtr();
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