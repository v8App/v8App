// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8InitApp.h"

#include "V8ContextProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8ContextProviderTest = V8InitApp;

        TEST_F(V8ContextProviderTest, CreateDisposeContext)
        {
            V8ContextProvider provider;
            JSRuntimeSharedPtr runtime = m_App->GetMainRuntime();

            EXPECT_EQ(nullptr, provider.CreateContext(runtime, "test", "NotExists", "", "", false, SnapshotMethod::kNamespaceOnly, 0));

            JSContextSharedPtr jsContext = provider.CreateContext(runtime, "test", "", "", "", false, SnapshotMethod::kNamespaceOnly, 0);
            V8LValue testValue;

            const char* source1 = R"(
                function test42() {
                    return 42;
                }
            )";

            jsContext->RunScript(source1);

            const char* source2 = R"(
                return function42();
            )";
            testValue = jsContext->RunScript(source2);
            EXPECT_EQ(42, testValue->IntegerValue(jsContext->GetLocalContext()).FromJust());
        }
    }
}