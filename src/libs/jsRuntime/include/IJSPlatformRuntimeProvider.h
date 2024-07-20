// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __IJSPLATFORM_RUNTIME_PROVIDER_H__
#define __IJSPLATFORM_RUNTIME_PROVIDER_H__

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Provides an interface for the platform get info from a runtime
         */
        class IJSPlatformRuntimeProvider
        {
        public:
            IJSPlatformRuntimeProvider() = default;
            virtual ~IJSPlatformRuntimeProvider() = default;
            virtual V8TaskRunnerSharedPtr GetForegroundTaskRunner(V8Isolate *inIsolate, V8TaskPriority priority) = 0;
            virtual bool IdleTasksEnabled(V8Isolate *inIsolate) = 0;
        };
    }
}

#endif //__IJSPLATFORM_RUNTIME_PROVIDER_H__