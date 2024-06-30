// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_SNAPSHOT_PROVIDER_H_
#define _JS_SNAPSHOT_PROVIDER_H_

#include <string>
#include <map>

#include "Assets/AppAssetRoots.h"

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that handles loading the snapshot data for the standard v8 data.
         * Ovderride this class to implement custom snapshots to handle the serialization custom format.
         */
        class V8SnapshotProvider
        {
        public:
            V8SnapshotProvider(std::filesystem::path inSnapshotPath = std::filesystem::path()) : m_SnapshotPath(inSnapshotPath)
            {
            }
            virtual ~V8SnapshotProvider() = default;

            /**
             * Load the snapshot data file.
             * Subclasses can overrride to pull additional data out that they may have added to the file
             */
            virtual bool LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath = std::filesystem::path());
            /**
             * Gets the v8 startup data that the isolate needs
             */
            virtual const V8StartupData *GetSnapshotData() { return &m_StartupData; };
            /**
             * Gets the file path that the data was loaded from
             */
            std::filesystem::path GetSnapshotPath() { return m_SnapshotPath; }
            /**
             * Has the snapshot data been loaded from the file yet
             */
            bool SnapshotLoaded() { return m_Loaded; }

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() { return nullptr; }

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) { return {nullptr, 0}; };
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) { return {nullptr, 0}; };

            /**
             * Deserializers for the snapshot
             */
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) {};
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload) {};

            /**
             * Returns the callback to pass to the context setup so
             * the internal callbacks are setup correctly below
             */
            v8::SerializeInternalFieldsCallback GetInternalSerializerCallaback();
            v8::SerializeContextDataCallback GetContextSerializerCallback();
            v8::DeserializeInternalFieldsCallback GetInternalDeserializerCallback();
            v8::DeserializeContextDataCallback GetContextDeserializerCallaback();

            /**
             * Internal serializers that get the provider and call the real serializer for it
             */
            static V8StartupData SerializeInternalField_Internal(V8LObject inHolder, int inIndex, void *inData);
            static V8StartupData SerializeContextInternalField_Internal(V8LContext inHolder, int inIndex, void *inData);

            static void DeserializeInternalField_Internal(V8LObject inHolder, int inIndex, V8StartupData inPayload, void *inData);
            static void DeserializeContextInternalField_Internal(V8LContext inHolder, int inIndex, V8StartupData inPayload, void *inData);

        protected:
            bool LoadDataFile(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath);
            /**
             * Has the data been loaded
             */
            bool m_Loaded = false;
            /**
             * Snapshot data
             */
            V8StartupData m_StartupData{nullptr, 0};
            /**
             * Path of teh file the data was loaded from
             */
            std::filesystem::path m_SnapshotPath;
        };
    }
}

#endif //_JS_SNAPSHOT_PROVIDER_H_