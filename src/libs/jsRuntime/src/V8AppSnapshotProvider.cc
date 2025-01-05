// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Format.h"

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
                LOG_ERROR(Utils::format("Passed snapshot path doesn't exist {}", m_SnapshotPath));
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

            std::string v8AppMagicNumber;
            uint32_t major_ver = 0;
            uint32_t minor_ver = 0;
            uint32_t patch_ver = 0;
            uint32_t build_ver = 0;

            // use the major version to read in the first int that would be
            //V8's magic number
            rBuffer >> major_ver;
            if (major_ver != 0)
            {
                LOG_ERROR("Magic Number in file is not 0");
                return false;
            }
            // that we should get a string for our magic number
            rBuffer >> v8AppMagicNumber;
            if (v8AppMagicNumber != kV8AppMagicNumber)
            {
                LOG_ERROR(Utils::format("Magic Number in file is not {}", kV8AppMagicNumber));
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


            rBuffer >> m_AppClassType;

            JSAppSharedPtr app = JSAppCreatorRegistry::CreateApp(m_AppClassType);
            if (app == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to find JSApp type {} registered with the JSAppCreator", m_AppClassType));
                return false;
            }
            m_SnapData = app->LoadSnapshotData(rBuffer);
            if (m_SnapData == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to load the snapshot data for JSAppType {}.", m_AppClassType));
                return false;
            }
            m_Loaded = true;
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
            return m_SnapData->m_RuntimesSnapIndexes.GetIndexForName(inRuntimeName);
        }

        size_t V8AppSnapshotProvider::GetIndexForContextName(std::string inName, std::string inRuntimeName)
        {
            size_t runtimeIndex = GetIndexForRuntimeName(inRuntimeName);
            return GetIndexForContextName(inName, runtimeIndex);
        }

        size_t V8AppSnapshotProvider::GetIndexForContextName(std::string inName, size_t inRuntimeIndex)
        {
            JSRuntimeSnapDataSharedPtr runtimeData = m_SnapData->m_RuntimesSnapData[inRuntimeIndex];
            return runtimeData->m_ContextIndexes.GetIndexForName(inName);
        }

        bool V8AppSnapshotProvider::IsRuntimeIndexValid(size_t inIndex)
        {
            return inIndex < m_SnapData->m_RuntimesSnapIndexes.GetNumberOfIndexes();
        }

        bool V8AppSnapshotProvider::IsContextIndexValid(size_t inIndex, std::string inRuntimeName)
        {
            size_t runtimeIndex = GetIndexForRuntimeName(inRuntimeName);
            return IsContextIndexValid(inIndex, runtimeIndex);
        }

        bool V8AppSnapshotProvider::IsContextIndexValid(size_t inIndex, size_t inRuntimeIndex)
        {
            JSRuntimeSnapDataSharedPtr runtimeData = m_SnapData->m_RuntimesSnapData[inRuntimeIndex];
            return inIndex < runtimeData->m_ContextIndexes.GetNumberOfIndexes();
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
            // TODO: Create a sytem simliar to the object info to handle dynamic modules wrting to internal fields
            // Serialization::ReadBuffer rBuffer(inPayload.data, inPayload.raw_size);
        }

        void V8AppSnapshotProvider::DeserializeAPIWrapperField(V8LObject inHolder, V8StartupData inPayload)
        {
            Serialization::ReadBuffer rBuffer(inPayload.data, inPayload.raw_size);

            std::string typeName;
            rBuffer >> typeName;
            if (rBuffer.HasErrored())
            {
                // Should throw an error here
                return;
            }
            CppBridge::V8CppObjInfo *info = CppBridge::CallbackRegistry::GetNativeObjectInfoFromTypeName(typeName);
            if (info == nullptr)
            {
                // should throw an error here
                return;
            }

            if (info == nullptr || info->m_Deserializer == nullptr)
            {
                // should probably throw an error
                return;
            }
            CppBridge::V8CppObjectBase *obj = info->m_Deserializer(inHolder->GetIsolate(), inHolder, rBuffer);
            if (obj == nullptr)
            {
                // should probablt throw an error
                return;
            }
            V8Object::Wrap<v8::CppHeapPointerTag::kDefaultTag>(inHolder->GetIsolate(), inHolder, obj);
        }

        void V8AppSnapshotProvider::DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload, JSContext *inJSContext)
        {
            Serialization::ReadBuffer rBuffer(inPayload.data, inPayload.raw_size);

            inJSContext->DeserializeContextData(inHolder, rBuffer);
        }

        JSAppSharedPtr V8AppSnapshotProvider::RestoreApp(std::filesystem::path inAppRoot, AppProviders inProviders)
        {
            if(m_AppRestored)
            {
                LOG_ERROR("The app has already been restored");
                return nullptr;
            }
            if(m_AppClassType.empty())
            {
                LOG_ERROR("The JSApp class type is empty");
                return nullptr;
            }
            JSAppSharedPtr app = JSAppCreatorRegistry::CreateApp(m_AppClassType);
            if (app == nullptr)
            {
                LOG_ERROR(Utils::format("Failed to find JSApp type {} registered with the JSAppCreator", m_AppClassType));
                return nullptr;
            }
            if(app->RestoreSnapshot(m_SnapData, inAppRoot, inProviders) == false)
            {
                app->DisposeApp();
                LOG_ERROR("Failed to restore the App from the snapshot");
                return nullptr;
            }
            m_AppRestored = true;
            return app;
        }
    }
}