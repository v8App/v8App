// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8CONTEXT_PROVIDER_H__
#define __V8CONTEXT_PROVIDER_H__

#include "IJSContextProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Implments the Context Creation hepler for JSContextes
         */
        class V8ContextProvider : public IJSContextProvider
        {
        public:
            V8ContextProvider() = default;
            virtual ~V8ContextProvider() = default;

            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
                                                     std::filesystem::path inEntryPoint, bool inSupportsSnapshot,
                                                     SnapshotMethod inSnapMethod, size_t inContextIndex, bool inUseV8Default) override;
            virtual void DisposeContext(JSContextSharedPtr inContext) override;
        };
    }
}

#endif //__V8CONTEXT_PROVIDER_H__