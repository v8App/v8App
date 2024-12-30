// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/WriteBuffer.h"
#include "Serialization/TypeSerializer.h"
#include "Utils/Format.h"

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8CppObjInfo.h"
#include "V8AppSnapshotCreator.h"
#include "JSContext.h"
#include "JSApp.h"
#include "JSRuntimeVersion.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool V8AppSnapshotCreator::CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile)
        {
            if (inSnapshotFile.empty())
            {
                LOG_ERROR("CreateSnapshot passed an empty file path");
                return false;
            }

            JSApp *app = dynamic_cast<JSApp *>(&inObject);
            if (app == nullptr)
            {
                LOG_ERROR("CreateSnapshot expected a JSApp Object");
                return false;
            }

            if (app->IsSnapshotApp() == false)
            {
                LOG_ERROR(Utils::format("The app {} is not snapshottable", app->GetName()));
                return false;
            }

            Serialization::WriteBuffer buffer;
            uint32_t v8AppMagicNumber = 0; // TODO: comeup with a magic number

            // V8's magic number is at the start we want it to be 0 so we can tell it's ours
            buffer << (uint32_t)0;
            buffer << v8AppMagicNumber;
            buffer << (uint32_t)JSRUNTIME_MAJOR_VERSION;
            buffer << (uint32_t)JSRUNTIME_MINOR_VERSION;
            buffer << (uint32_t)JSRUNTIME_PATCH_LEVEL;
            buffer << (uint32_t)JSRUNTIME_BUILD_NUM;

            buffer << "PlatformArch"; // TODO: Turn into some macro or function to return a string for the arch and platform

            buffer << app->GetClassType();

            if (app->MakeSnapshot(buffer) == false)
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
            //TODO: Add a system similiar for the ObjInfo being stored in an internal field
            //for dynamic modules to be able to set info
            //Serialization::WriteBuffer wBuffer;

            return {nullptr, 0};
        }

        V8StartupData V8AppSnapshotCreator::SerializeAPIWrapperField(V8LObject inHolder, CppBridge::V8CppObjectBase* inObject)
        {
            Serialization::WriteBuffer wBuffer;

            const CppBridge::V8CppObjInfo& info = inObject->GetTypeInfo();
            if(info.m_Serializer != nullptr)
            {
                wBuffer << info.m_TypeName;
                info.m_Serializer(wBuffer, inObject);
                if(wBuffer.HasErrored() == false)
                {
                    return {wBuffer.GetDataNew(), (int)wBuffer.BufferSize()};
                }
            }
            return {nullptr, 0};
        }

        V8StartupData V8AppSnapshotCreator::SerializeContextInternalField(V8LContext inHolder, int inIndex)
        {
            if (inIndex == (int)JSContext::DataSlot::kJSContextWeakPtr)
            {
                Serialization::WriteBuffer wBuffer;

                JSContext *jsContext = static_cast<JSContext *>(inHolder->GetAlignedPointerFromEmbedderData(inIndex));
                jsContext->SerializeContextData(wBuffer);

                return {wBuffer.GetDataNew(), (int)wBuffer.BufferSize()};
            }
            return {nullptr, 0};
        }

    }
}