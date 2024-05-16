// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __JS_SNAPSHOT_CREATOR_H__
#define __JS_SNAPSHOT_CREATOR_H__

#include <filesystem>

#include "V8Types.h"
#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        class JSSnapshotCreator
        {
        public:
            JSSnapshotCreator(JSAppSharedPtr inApp);
            ~JSSnapshotCreator();

            bool CreateSnapshotFile(std::filesystem::path inSnapshotFile);

            static v8::StartupData SerializeInternalField(V8LocalObject inHolder, int inIndex, void *inData);
            static v8::StartupData SerializeContextInternalField(V8LocalContext inHolder, int inIndex, void *inData);

            static void DeserializeInternalField(V8LocalObject inHolder, int inINdex, v8::StartupData inPayload, void *inData);            
            static void DeserializeContextInternalField(V8LocalContext inHolder, int inINdex, v8::StartupData inPayload, void *inData);            
        protected:
            JSAppSharedPtr m_App;
        };
    }
}

#endif //__JS_SNAPSHOT_CREATOR_H__