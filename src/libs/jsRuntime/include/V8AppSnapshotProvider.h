// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8APP_SNAPSHOT_PROVIDER_H_
#define _V8APP_SNAPSHOT_PROVIDER_H_

#include <string>

#include "V8Types.h"
#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that handles loading the snapshot data for the standard v8 data.
         * Ovderride this class to implement custom snapshots to handle the serialization custom format.
         *
         * File Format
         * |   4 Bytes    | 0 used to detect if it's a v8 created snapshot since it has a magic number at the start
         * |   4 Bytes    | Magic Number for v8App
         * |   4 Bytes    | Runtime Major Version
         * |   4 Bytes    | Runtime Minor Version
         * |   4 Bytes    | Runtime Patch Version
         * |   4 Bytes    | Runtime Build Version
         * | 4 or 8 bytes | length of architecture
         * |     ...      | content of architecture string
         * | 4 or 8 bytes | length of JSApp class type
         * |     ...      | content of JSApp class type string
         * |     ...      | JSApp Data
         * |     ...      |     Runtime NamedIndexes the number that are serialized is number of snapshot below
         * ---------------- Snapshot Section
         * | 4 or 8 bytes | snapshot blob size
         * |     ...      | v8 snapshot blob created by the app
         * |     ...      |
         * |     ...      | JSRuntime data
         * |     ...      |     COntext NamedIndex
         * ----------------
         * |     ...      | n snapshot
         * ----------------
         */
        class V8AppSnapshotProvider : public IJSSnapshotProvider
        {
        public:
            V8AppSnapshotProvider() = default;
            virtual ~V8AppSnapshotProvider() = default;

            virtual bool LoadSnapshotData(std::filesystem::path = std::filesystem::path()) override;

            /**
             * Gets the v8 startup data that the isolate needs
             */
            virtual const V8StartupData *GetSnapshotData(size_t inIndex = 0) override;

            /**
             * Gets the index for the runtime name. Returns MaxSupported names if it doesn't exist
             * Use the IsRuntimeIndexValid to check if the index is valid
             */
            virtual size_t GetIndexForRuntimeName(std::string inRuntimeName) override;

            /**
             * Gets the index for the context name. Returns MaxSupported names if it doesn't exist
             * Use the IsContextIndexValid to check if the index is valid
             */
            virtual size_t GetIndexForContextName(std::string inName, std::string inRuntimeName) override;
            virtual size_t GetIndexForContextName(std::string inName, size_t inRuntimeIndex) override;
            /**
             * Checks if the passed index is valid
             */
            virtual bool IsRuntimeIndexValid(size_t inIndex) override;
            /**
             * Checks if the passed index is valid
             */
            virtual bool IsContextIndexValid(size_t inIndex, std::string inRuntimeName) override;
            virtual bool IsContextIndexValid(size_t inIndex, size_t inRuntimeIndex) override;

            /**
             * V8App adds 1 to the real index of the contexts since V8 starts at 0 for non default ones
             */
            virtual size_t RealContextIndex(size_t inNamedIndex) override { return inNamedIndex - 1; };

            virtual const intptr_t *GetExternalReferences() override;
            /**
             * Returns the JSApp snaoshot data
             */
            virtual const JSAppSnapDataSharedPtr GetJSAppSnapData() override { return m_SnapData; }

            // creates the app from the snapshot data
            JSAppSharedPtr CreateApp();

            /**
             * Deserializers for the snapshot
             */
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) override;
            virtual void DeserializeAPIWrapperField(V8LObject inHolder, V8StartupData inPayload) override;
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload, JSContext* inJSContext) override;

        protected:
            JSAppSnapDataSharedPtr m_SnapData;
        };

        using V8AppSnapshotProviderSharedPtr = std::shared_ptr<V8AppSnapshotProvider>;
    }
}

#endif //_V8APP_SNAPSHOT_PROVIDER_H_