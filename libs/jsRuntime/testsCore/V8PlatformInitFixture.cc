// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tools/cpp/runfiles/runfiles.h"

#include "Utils/Environment.h"

#include "V8PlatformInitFixture.h"

namespace v8App
{
    namespace JSRuntime
    {
        std::unique_ptr<InitializeV8Testing> s_InitSingleton;
    }
}