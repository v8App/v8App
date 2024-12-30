// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _IJSCONTEXT_PROVIDER_H__
#define _IJSCONTEXT_PROVIDER_H__

#include <map>
#include <filesystem>

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /*
         * Used to isolate the context creation during testing and potentially different JS engines
         */
        class IJSContextProvider
        {
        public:
            IJSContextProvider() = default;
            virtual ~IJSContextProvider() = default;
            /**
             * Override to create a v8 context
             */
            virtual JSContextSharedPtr CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespce,
                                                     std::filesystem::path inEntryPoint, bool inSupportsSnapshot,
                                                     SnapshotMethod inSnapMethod, size_t inContextIndex) = 0;
            /**
             * Override to dispose of the v8 context
             */
            virtual void DisposeContext(JSContextSharedPtr inContext) = 0;
        };

    }
}
#endif //_JSCONTEXT_PROVIDER_H__