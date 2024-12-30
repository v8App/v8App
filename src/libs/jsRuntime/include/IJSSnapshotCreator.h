// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _IJSSNAPSHOT_CREATOR_H__
#define _IJSSNAPSHOT_CREATOR_H__

#include <filesystem>

#include "V8Types.h"
#include "ISnapshotObject.h"
#include "CppBridge/V8CppObjBase.h"

namespace v8App
{
    namespace JSRuntime
    {
        /*
         * Used to isolate the context creation during testing and potentially different JS engines
         */
        class IJSSnapshotCreator
        {
        public:
            IJSSnapshotCreator() = default;
            virtual ~IJSSnapshotCreator() = default;

            virtual bool CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile) = 0;

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() = 0;

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) = 0;
            virtual V8StartupData SerializeAPIWrapperField(V8LObject inHolder, CppBridge::V8CppObjectBase* inObject) = 0;
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) = 0;

            /**
             * Returns the callback to pass to the context setup so
             * the internal callbacks are setup correctly below
             */
            v8::SerializeInternalFieldsCallback GetInternalSerializerCallaback();
            v8::SerializeAPIWrapperCallback GetAPIWrapperSerializerCallaback();
            v8::SerializeContextDataCallback GetContextSerializerCallback();

            /**
             * Internal serializers that get the provider and call the real serializer for it
             */
            static V8StartupData SerializeInternalField_Internal(V8LObject inHolder, int inIndex, void *inData);
            static V8StartupData SerializeAPIWrapperField_Internal(V8LObject inHolder, void* inCppObject, void *inData);
            static V8StartupData SerializeContextInternalField_Internal(V8LContext inHolder, int inIndex, void *inData);
        };

    }
}
#endif //_IJSSNAPSHOT_CREATOR_H__