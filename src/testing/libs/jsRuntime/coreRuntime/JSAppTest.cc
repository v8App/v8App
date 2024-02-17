// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "JSApp.h"

namespace v8App
{
    namespace JSRuntime
    {
        TEST(JSAppTest, ConstrcutorInitializeDispose)
        {
            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            app->Initialize();
            EXPECT_EQ("test", app->GetName());
            EXPECT_NE(nullptr, app->GetCodeCache());
            EXPECT_NE(nullptr, app->GetAppRoots());
            app->DisposeApp();
        }

        TEST(JSAppTest, GetCreateJSRuntime)
        {
            JSAppSharedPtr app = std::make_shared<JSApp>("test");
            app->Initialize();

            std::string runtimeName = "AppJSRtuntime";

            EXPECT_EQ(nullptr, app->GetJSRuntimeByName(runtimeName));

            JSRuntimeSharedPtr runtime = app->CreateJSRuntime(runtimeName);
            EXPECT_NE(nullptr, runtime);
            EXPECT_EQ(nullptr, app->CreateJSRuntime(runtimeName));
            JSRuntimeSharedPtr runtime2 = app->CreateJSRuntimeOrGet(runtimeName);
            EXPECT_EQ(runtime2, runtime);
            runtime2 = app->CreateJSRuntimeOrGet("AppJSRuntime2");
            EXPECT_NE(nullptr, runtime2);
            app->DisposeApp();
        }
    }
}