// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Logging/LogMacros.h"
#include "Utils/Paths.h"

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Serialization/TypeSerializer.h"

#include "CppBridge/V8CppObjInfo.h"
#include "JSContext.h"
#include "JSRuntime.h"
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

        v8::StartupData V8AppSnapshotProvider::SerializeInternalField(V8LocalObject inHolder, int inIndex)
        {
            Serialization::WriteBuffer wBuffer;

            // Index 0 is hte object info but we can't do anythign with it without the actual cpp object
            if (inIndex < (int)V8CppObjDataIntField::ObjInstance)
            {
                CppBridge::V8CppObjInfo *info = CppBridge::V8CppObjInfo::From(inHolder);
                if (info == nullptr)
                {
                    // should prboably thow an error here
                    return {nullptr, 0};
                }
                wBuffer << info->m_TypeName;
                if (wBuffer.HasErrored())
                {
                    // should throw an error here
                    return {nullptr, 0};
                }
                return {wBuffer.GetDataNew(), (int)wBuffer.BufferSize()};
            }
            if (inIndex == (int)V8CppObjDataIntField::ObjInstance)
            {
                CppBridge::V8CppObjInfo *info = CppBridge::V8CppObjInfo::From(inHolder);
                CppBridge::V8NativeObjectBase *instance = static_cast<CppBridge::V8NativeObjectBase *>(inHolder->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInstance));
                if (info->m_Serializer != nullptr)
                {
                    info->m_Serializer(wBuffer, instance);
                    if (wBuffer.HasErrored())
                    {
                        // should thorw and error here
                        return { nullptr, 0};
                    }
                    return {wBuffer.GetDataNew(), (int)wBuffer.BufferSize()};
                }
            }
            return {nullptr, 0};
        }

        v8::StartupData V8AppSnapshotProvider::SerializeContextInternalField(V8LocalContext inHolder, int inIndex)
        {
            if (inIndex == (int)JSContext::DataSlot::kJSContextWeakPtr)
            {
                JSContext *jsContext = static_cast<JSContext *>(inHolder->GetAlignedPointerFromEmbedderData(inIndex));
                // TODO serialize the context
            }
            return {nullptr, 0};
        }

        void V8AppSnapshotProvider::DeserializeInternalField(V8LocalObject inHolder, int inIndex, v8::StartupData inPayload)
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
                CppBridge::V8NativeObjectBase *obj = objInfo->m_Deserializer(inHolder->GetIsolate(), inHolder, rBuffer);
                if (obj == nullptr)
                {
                    // should probablt throw an error
                    return;
                }
                inHolder->SetAlignedPointerInInternalField((int)V8CppObjDataIntField::ObjInstance, obj);
            }
        }

        void V8AppSnapshotProvider::DeserializeContextInternalField(V8LocalContext inHolder, int inIndex, v8::StartupData inPayload)
        {
            // todo deserialize the context
        }
    }
}