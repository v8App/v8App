// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JS_CONTEXT_CREATION_H_
#define _JS_CONTEXT_CREATION_H_

#include <unordered_map>

#include "JSRuntime.h"
#include "JSContext.h"
#include "JSContextCreationHelper.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Implments the Context Creation hepler for JSContextes
         */
        class JSContextCreator : public JSContextCreationHelper
        {
        public:
            JSContextCreator() = default;
            virtual ~JSContextCreator() = default;
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
            std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot, 
                      SnapshotMethod inSnapMethod) override;
            virtual void DisposeContext(JSContextSharedPtr inContext) override;
            virtual void RegisterSnapshotCloser(JSContextSharedPtr inContext) override;
            virtual void UnregisterSnapshotCloser(JSContextSharedPtr inContext) override;
        };
    }
}

#endif //_JS_CONTEXT_CREATION_H_