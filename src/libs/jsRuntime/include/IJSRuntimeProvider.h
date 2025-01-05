// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _IJSRUNTIME_PROVIDER_H__
#define _IJSRUNTIME_PROVIDER_H__

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class that isolates the rutime creation for test and potentially different JS engines
         */
        class IJSRuntimeProvider
        {
        public:
            IJSRuntimeProvider() = default;
            virtual ~IJSRuntimeProvider() = default;

            virtual JSRuntimeSharedPtr CreateRuntime() = 0;
            /**
             * Override to dispose of the JSRuntime
             */
            virtual void DisposeRuntime(JSRuntimeSharedPtr inRuntime) = 0;
        };
    }
}

#endif //_IJSRUNTIME_PROVIDER_H__