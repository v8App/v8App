// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "v8.h"
#include "ScriptStartupDataManager.h"

namespace v8App
{
    namespace JSRuntime
    {
        //TODO: Replace with a genrule to generate this file.
        v8::StartupData *ScriptStartupDataManager::GetStartupData()
        {
            return nullptr;
        }
    }
}