// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "IJSSnapshotProvider.h"
#include "JSContext.h"
#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace internal
        {
            struct ContextData
            {
                IJSSnapshotProvider *provider;
                JSContext *jsContext;
            };
        }

        v8::DeserializeInternalFieldsCallback IJSSnapshotProvider::GetInternalDeserializerCallback()
        {
            return v8::DeserializeInternalFieldsCallback(IJSSnapshotProvider::DeserializeInternalField_Internal, this);
        }

        v8::DeserializeContextDataCallback IJSSnapshotProvider::GetContextDeserializerCallaback(JSContext *inJSContext)
        {
            return v8::DeserializeContextDataCallback(IJSSnapshotProvider::DeserializeContextInternalField_Internal, inJSContext);
        }

        void IJSSnapshotProvider::DeserializeInternalField_Internal(V8LObject inHolder, int inIndex, V8StartupData inPayload, void *inData)
        {
            IJSSnapshotProvider *provider = static_cast<IJSSnapshotProvider*>(inData);
            if (provider != nullptr)
            {
                provider->DeserializeInternalField(inHolder, inIndex, inPayload);
            }
        }

        void IJSSnapshotProvider::DeserializeContextInternalField_Internal(V8LContext inHolder, int inIndex, V8StartupData inPayload, void *inData)
        {
            JSContext *jsContext = static_cast<JSContext *>(inData);
            IJSSnapshotProviderSharedPtr provider = jsContext->GetJSApp()->GetSnapshotProvider();

            if (provider != nullptr)
            {
                provider->DeserializeContextInternalField(inHolder, inIndex, inPayload, jsContext);
            }
        }
    }
}
