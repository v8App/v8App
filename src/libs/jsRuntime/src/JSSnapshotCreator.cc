// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Utils/Format.h"

#include "JSSnapshotCreator.h"
#include "JSContext.h"

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
        }

        v8::StartupData JSSnapshotCreator::SerializeContextInternalField(V8LocalContext inHolder, int inIndex, void *inData)
        {
            if (inIndex == int(ContextDataSlot::kJSContextWeakPtr))
            {
                JSContextWeakPtr *ptr = static_cast<JSContextWeakPtr *>(inHolder->GetAlignedPointerFromEmbedderData(inIndex));
                //TODO: Serialize the JSContext
            }
            return {nullptr, 0};
        }

        void JSSnapshotCreator::DeserializeInternalField(V8LocalObject inHolder, int inINdex, v8::StartupData inPayload, void *inData)
        {
        }

        void JSSnapshotCreator::DeserializeContextInternalField(V8LocalContext inHolder, int inINdex, v8::StartupData inPayload, void *inData)
        {
        }
    }
}