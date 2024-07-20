// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        v8::SerializeInternalFieldsCallback IJSSnapshotProvider::GetInternalSerializerCallaback()
        {
            return v8::SerializeInternalFieldsCallback(&IJSSnapshotProvider::SerializeInternalField_Internal, this);
        }

        v8::SerializeContextDataCallback IJSSnapshotProvider::GetContextSerializerCallback()
        {
            return v8::SerializeContextDataCallback(&IJSSnapshotProvider::SerializeContextInternalField_Internal, this);
        }

        v8::DeserializeInternalFieldsCallback IJSSnapshotProvider::GetInternalDeserializerCallback()
        {
            return v8::DeserializeInternalFieldsCallback(IJSSnapshotProvider::DeserializeInternalField_Internal, this);
        }

        v8::DeserializeContextDataCallback IJSSnapshotProvider::GetContextDeserializerCallaback()
        {
            return v8::DeserializeContextDataCallback(IJSSnapshotProvider::DeserializeContextInternalField_Internal, this);
        }

        V8StartupData IJSSnapshotProvider::SerializeInternalField_Internal(V8LObject inHolder, int inIndex, void *inData)
        {
            IJSSnapshotProvider *provider = static_cast<IJSSnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->SerializeInternalField(inHolder, inIndex);
            }
            return {nullptr, 0};
        }

        V8StartupData IJSSnapshotProvider::SerializeContextInternalField_Internal(V8LContext inHolder, int inIndex, void *inData)
        {
            IJSSnapshotProvider *provider = static_cast<IJSSnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->SerializeContextInternalField(inHolder, inIndex);
            }
            return {nullptr, 0};
        }

        void IJSSnapshotProvider::DeserializeInternalField_Internal(V8LObject inHolder, int inIndex, V8StartupData inPayload, void *inData)
        {
            IJSSnapshotProvider *provider = static_cast<IJSSnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->DeserializeInternalField(inHolder, inIndex, inPayload);
            }
        }

        void IJSSnapshotProvider::DeserializeContextInternalField_Internal(V8LContext inHolder, int inIndex, V8StartupData inPayload, void *inData)
        {
            IJSSnapshotProvider *provider = static_cast<IJSSnapshotProvider *>(inData);
            if (provider != nullptr)
            {
                provider->DeserializeContextInternalField(inHolder, inIndex, inPayload);
            }
        }
    }
}
