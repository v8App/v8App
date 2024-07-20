// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_SNAPSHOT_PROVIDER_H_
#define _JS_SNAPSHOT_PROVIDER_H_

#include <string>
#include <filesystem>

#include "Assets/BinaryAsset.h"

#include "V8Types.h"
#include "IJSSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that handles loading the snapshot data for the standard v8 data.
         */
        class V8SnapshotProvider : public IJSSnapshotProvider
        {
        public:
            V8SnapshotProvider() = default;
            virtual ~V8SnapshotProvider() = default;

            /**
             * Load the snapshot data file.
             * Subclasses can overrride to pull additional data out that they may have added to the file
             */
            virtual bool LoadSnapshotData(JSAppSharedPtr inApp, std::filesystem::path inSnapshotPath) override;

            /**
             * Gets the v8 startup data that the isolate needs
             * Passing an index is ignored as there is only one snapshot for v8
             */
            virtual const V8StartupData *GetSnapshotData(size_t inIndex = 0) override { return &m_StartupData; };

            /**
             * Gets the external references that the snapshot will need.
             * Default v8 doesn't need any
             */
            virtual const intptr_t *GetExternalReferences() override { return nullptr; }

            /**
             * Serializers for the snapshot
             */
            virtual V8StartupData SerializeInternalField(V8LObject inHolder, int inIndex) override { return {nullptr, 0}; };
            virtual V8StartupData SerializeContextInternalField(V8LContext inHolder, int inIndex) override { return {nullptr, 0}; };

            /**
             * Deserializers for the snapshot
             */
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) override {};
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload) override {};

        protected:
            /**
             * Asset File for the data
             */
            Assets::BinaryAsset m_AssetFile;
            /**
             * Snapshot data
             */
            V8StartupData m_StartupData{nullptr, 0};
        };
    }
}

#endif //_JS_SNAPSHOT_PROVIDER_H_