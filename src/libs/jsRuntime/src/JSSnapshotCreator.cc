// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Utils/Format.h"

#include "JSSnapshotCreator.h"
#include "JSContext.h"
#include "CppBridge/V8NativeObject.h"
#include "CppBridge/CallbackRegistry.h"

namespace v8App
{
    namespace JSRuntime
    {

        JSSnapshotCreator::JSSnapshotCreator(JSAppSharedPtr inApp) : m_App(inApp)
        {
        }

        JSSnapshotCreator::~JSSnapshotCreator()
        {
        }

        bool JSSnapshotCreator::CreateSnapshotFile(std::filesystem::path inSnapshotFile)
        {
            V8SnapshotCreatorSharedPtr creator = m_App->GetSnapshotCreator();
            if (creator == nullptr)
            {
                return false;
            }

            JSContextSharedPtr defaultContext = m_App->CreateJSContext("default");
            {
                v8::HandleScope hScope(m_App->GetJSRuntime()->GetIsolate());

                V8LocalContext lContext = defaultContext->GetLocalContext();

                creator->SetDefaultContext(lContext, v8::SerializeInternalFieldsCallback(&JSSnapshotCreator::SerializeInternalField),
                v8::SerializeContextDataCallback(JSSnapshotCreator::SerializeContextInternalField));
                m_App->GetJSRuntime()->CloseOpenHandlesForSnapshot();
            }

            {
                // V8Platform::Get()->SetWorkersPaused(true);
                v8::StartupData data = creator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);
                if (data.raw_size == 0)
                {
                    V8Platform::Get()->SetWorkersPaused(false);
                    return false;
                }
                // V8Platform::Get()->SetWorkersPaused(false);

                std::ofstream snapFile(inSnapshotFile, std::ios::out | std::ios::binary);
                if (snapFile.is_open() == false || snapFile.fail())
                {
                    return false;
                }
                snapFile.write(data.data, data.raw_size);
                snapFile.close();
            }
            return true;
          }

        v8::StartupData JSSnapshotCreator::SerializeInternalField(V8LocalObject inHolder, int inIndex, void *inData)
        {
            //Index 0 is hte object info but we can't do anythign with it without the actual cpp object
            if(inIndex == 0)
            {
                return {nullptr, 0};
            }
            if(inIndex == 1)
            {
                CppBridge::V8NativeObjectInfo* info = CppBridge::V8NativeObjectInfo::From(inHolder);
                CppBridge::V8NativeObjectBase* instance = static_cast<CppBridge::V8NativeObjectBase*>(inHolder->GetAlignedPointerFromInternalField(CppBridge::V8NativeObjectInternalFields::kV8NativeObjectInstance));
                if(info->m_Serializer != nullptr)
                {
                    return info->m_Serializer(instance);
                }
            }
            return {nullptr, 0};
        }

        v8::StartupData JSSnapshotCreator::SerializeContextInternalField(V8LocalContext inHolder, int inIndex, void *inData)
        {
            if (inIndex == int(ContextDataSlot::kJSContextWeakPtr))
            {
                JSContext *ptr = static_cast<JSContext*>(inHolder->GetAlignedPointerFromEmbedderData(inIndex));
                //TODO: Serialize the JSContext
            }
            return {nullptr, 0};
        }

        void JSSnapshotCreator::DeserializeInternalField(V8LocalObject inHolder, int inIndex, v8::StartupData inPayload, void *inData)
        {
            if(inIndex == 0)
            {
                std::string typeName(inPayload.data, inPayload.raw_size);
                if(typeName.empty())
                {
                    return;
                }
                CppBridge::V8NativeObjectInfo* info = CppBridge::CallbackRegistry::GetNativeObjectInfoFromTypeName(typeName);
                if(info == nullptr)
                {
                    return;
                }
                inHolder->SetAlignedPointerInInternalField(inIndex, info);
            }
            if(inIndex == 1)
            {
                CppBridge::V8NativeObjectInfo * info = CppBridge::V8NativeObjectInfo::From(inHolder);
                if(info->m_Deserializer != nullptr)
                {
                    CppBridge::V8NativeObjectBase *instance = info->m_Deserializer(inHolder->GetIsolate(), inHolder, inPayload);
                    inHolder->SetAlignedPointerInInternalField(inIndex, instance);
                }
            }
        }

        void JSSnapshotCreator::DeserializeContextInternalField(V8LocalContext inHolder, int inINdex, v8::StartupData inPayload, void *inData)
        {
        }
    }
}