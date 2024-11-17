// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8RuntimeProvider.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntimeSharedPtr V8RuntimeProvider::CreateRuntime()
        {
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>();
            return runtime;
        }

        void V8RuntimeProvider::DisposeRuntime(JSRuntimeSharedPtr inRuntime)
        {
            inRuntime->DisposeRuntime();
        }
    }
}