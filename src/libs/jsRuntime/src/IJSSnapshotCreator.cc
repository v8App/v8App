// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "IJSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        v8::SerializeInternalFieldsCallback IJSSnapshotCreator::GetInternalSerializerCallaback()
        {
            return v8::SerializeInternalFieldsCallback(&IJSSnapshotCreator::SerializeInternalField_Internal, this);
        }

        v8::SerializeAPIWrapperCallback IJSSnapshotCreator::GetAPIWrapperSerializerCallaback()
        {
            return v8::SerializeAPIWrapperCallback(&IJSSnapshotCreator::SerializeAPIWrapperField_Internal, this);
        }

        v8::SerializeContextDataCallback IJSSnapshotCreator::GetContextSerializerCallback()
        {
            return v8::SerializeContextDataCallback(&IJSSnapshotCreator::SerializeContextInternalField_Internal, this);
        }

        V8StartupData IJSSnapshotCreator::SerializeInternalField_Internal(V8LObject inHolder, int inIndex, void *inData)
        {
            IJSSnapshotCreator *provider = static_cast<IJSSnapshotCreator *>(inData);
            if (provider != nullptr)
            {
                return provider->SerializeInternalField(inHolder, inIndex);
            }
            return {nullptr, 0};
        }

        V8StartupData IJSSnapshotCreator::SerializeAPIWrapperField_Internal(V8LObject inHolder, void* inCppObject, void *inData)
        {
            IJSSnapshotCreator *provider = static_cast<IJSSnapshotCreator *>(inData);
            CppBridge::V8CppObjectBase* base = static_cast<CppBridge::V8CppObjectBase*>(inCppObject);
            if (provider != nullptr && base != nullptr)
            {
                return provider->SerializeAPIWrapperField(inHolder, base);
            }
            return {nullptr, 0};
        }

        V8StartupData IJSSnapshotCreator::SerializeContextInternalField_Internal(V8LContext inHolder, int inIndex, void *inData)
        {
            IJSSnapshotCreator *provider = static_cast<IJSSnapshotCreator *>(inData);
            if (provider != nullptr)
            {
                return provider->SerializeContextInternalField(inHolder, inIndex);
            }
            return {nullptr, 0};
        }
    }
}
