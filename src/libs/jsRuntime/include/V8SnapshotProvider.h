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
            virtual bool LoadSnapshotData(std::filesystem::path inSnapshotPath = std::filesystem::path()) override;
            /**
             * Always default 0 since there is only ever one runtime snapshot
             */
            virtual size_t GetIndexForRuntimeName(std::string inRuntimeName) override { return 0; };
            /**
             * The Stadnard V8 Sanpshot just has the default context
             */
            virtual size_t GetIndexForContextName(std::string inName, std::string inRuntimeName) override
            {
                return 0;
            };
            virtual size_t GetIndexForContextName(std::string inName, size_t inRuntimeIndex) override
            {
                return 0;
            };

            /**
             * The standard v8 index has only the one isolate so 0 is the only valid index
             */
            virtual bool IsRuntimeIndexValid(size_t inIndex) override { return inIndex == 0; }
            /**
             * The Stadnard V8 Sanpshot just has the default context so 0 is the only valid index
             */
            virtual bool IsContextIndexValid(size_t inIndex, std::string inRuntimeName) override
            {
                return inIndex == 0 && inRuntimeName == "";
            }
            virtual bool IsContextIndexValid(size_t inIndex, size_t inRuntimeIndex) override
            {
                return inIndex == 0 && inRuntimeIndex == 0;
            }

            /**
             * V8 Snapshot provider has no other contexts than the defualt V8 one which is
             * always 0
             */
            virtual size_t RealContextIndex(size_t inNamedIndex) override { return 0; };

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
             * The Standard V8 Snapshot has no JSApp snapshot data.
             */
            virtual const JSAppSnapDataSharedPtr GetJSAppSnapData() override { return {}; }

            /**
             * Deserializers for the snapshot
             */
            virtual void DeserializeInternalField(V8LObject inHolder, int inIndex, V8StartupData inPayload) override {};
            virtual void DeserializeAPIWrapperField(V8LObject inHolder, V8StartupData inPayload) override {};
            virtual void DeserializeContextInternalField(V8LContext inHolder, int inIndex, V8StartupData inPayload, JSContext *inJSContext) override {};

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