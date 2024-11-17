// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _V8RUNTIME_PROVIDER_H__
#define _V8RUNTIME_PROVIDER_H__

#include "IJSRuntimeProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        class V8RuntimeProvider : public IJSRuntimeProvider
        {
        public:
            V8RuntimeProvider() = default;
            ~V8RuntimeProvider() = default;

            virtual JSRuntimeSharedPtr CreateRuntime() override;
            /**
             * Override to dispose of the JSRuntime
             */
            virtual void DisposeRuntime(JSRuntimeSharedPtr inRuntime) override;
        };
    }
}

#endif //_IJSRUNTIME_PROVIDER_H__