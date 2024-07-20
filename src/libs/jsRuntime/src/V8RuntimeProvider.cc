// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8RuntimeProvider.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSRuntimeSharedPtr V8RuntimeProvider::CreateRuntime(JSAppSharedPtr inApp, std::string inName,
                                                            IdleTaskSupport inEnableIdleTasks,
                                                            bool inSetupForSnapshot,
                                                            size_t inRuntimeIndex,
                                                            AppProviders inAppProvider)
        {
            JSRuntimeSharedPtr runtime = std::make_shared<JSRuntime>(inApp, inEnableIdleTasks, inName, inSetupForSnapshot, inRuntimeIndex);
            if(runtime->Initialize(inAppProvider) == false)
            {
                return nullptr;
            }

            return runtime;
        }

        void V8RuntimeProvider::DisposeRuntime(JSRuntimeSharedPtr inRuntime)
        {
            inRuntime->DisposeRuntime();
        }
    }
}