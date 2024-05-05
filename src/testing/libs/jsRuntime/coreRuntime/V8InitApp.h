// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "TestSnapshotProvider.h"

#include "Utils/Environment.h"

#include "V8Platform.h"
#include "JSRuntime.h"
#include "JSContext.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace v8App
{
    namespace JSRuntime
    {
        class V8InitApp : public testing::Test
        {
        public:
            V8InitApp()
            {
                std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
                m_App = std::make_shared<JSApp>("testCore", snapProvider);
                m_App->InitializeRuntime(s_TestDir, "");
            }
            ~V8InitApp()
            {
                m_App->DisposeApp();
                m_App.reset();
            }

            JSAppSharedPtr m_App;
        };

    }
}