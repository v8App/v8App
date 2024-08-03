// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/WriteBuffer.h"
#include "Serialization/TypeSerializer.h"

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8CppObjInfo.h"
#include "V8AppSnapshotCreator.h"
#include "JSContext.h"
#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool V8AppSnapshotCreator::CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile)
        {
            if(inSnapshotFile.empty())
            {
                LOG_ERROR("CreateSnapshot passed an empty file path");
                return false;
            }

            JSApp* app = dynamic_cast<JSApp*>(&inObject);
            if(app == nullptr)
            {
                LOG_ERROR("CreateSnapshot expected a JSApp Object");
                return false;
            }

            Serialization::WriteBuffer buffer;

            if(app->MakeSnapshot(buffer) == false)
            {
                LOG_ERROR(Utils::format("Faile to snapshot the app {}", app->GetName()));
                return false;
            }

            std::ofstream snapFile(inSnapshotFile, std::ios::out | std::ios::binary);
            if (snapFile.is_open() == false || snapFile.fail())
            {
                return false;
            }
            snapFile.write(buffer.GetData(), buffer.BufferSize());
            snapFile.close();

            return true;
        }

        const intptr_t *V8AppSnapshotCreator::GetExternalReferences()
        {
            return CppBridge::CallbackRegistry::GetReferences().data();
        }

        V8StartupData V8AppSnapshotCreator::SerializeInternalField(V8LObject inHolder, int inIndex)
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
                CppBridge::V8CppObjectBase *instance = static_cast<CppBridge::V8CppObjectBase *>(inHolder->GetAlignedPointerFromInternalField((int)V8CppObjDataIntField::ObjInstance));
                if (info->m_Serializer != nullptr)
                {
                    info->m_Serializer(wBuffer, instance);
                    if (wBuffer.HasErrored())
                    {
                        // should thorw and error here
                        return {nullptr, 0};
                    }
                    return {wBuffer.GetDataNew(), (int)wBuffer.BufferSize()};
                }
            }
            return {nullptr, 0};
        }

        V8StartupData V8AppSnapshotCreator::SerializeContextInternalField(V8LContext inHolder, int inIndex)
        {
            if (inIndex == (int)JSContext::DataSlot::kJSContextWeakPtr)
            {
                JSContext *jsContext = static_cast<JSContext *>(inHolder->GetAlignedPointerFromEmbedderData(inIndex));
                // TODO serialize the context
            }
            return {nullptr, 0};
        }

    }
}