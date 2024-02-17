// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"

#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSContextModulesDeathTest = V8Fixture;

        class TestJSContextDeathModules : public JSContextModules
        {
        public:
            TestJSContextDeathModules(JSContextSharedPtr inContext) : JSContextModules(inContext) {}

            JSModuleInfoSharedPtr TestLoadModuleTree(JSContextSharedPtr inContext, JSModuleInfoSharedPtr inModInfo) { return LoadModuleTree(inContext, inModInfo); }
        };

        TEST_F(JSContextModulesDeathTest, Constructor)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSContextSharedPtr context = nullptr;
                JSContextModules modules(context);
                std::exit(0);
            },
                         "");
        }

        TEST_F(JSContextModulesDeathTest, LoadModuleTreeUnkownType)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                std::filesystem::path appRoot = m_App->GetAppRoots()->GetAppRoot();
                JSModuleInfoSharedPtr info = std::make_shared<JSModuleInfo>(m_Context);
                JSModuleInfo::AssertionInfo assertInfo;
                assertInfo.m_Type = JSModuleInfo::ModuleType::kInvalid;
                assertInfo.m_TypeString = "invalid";
                info->SetAssertionInfo(assertInfo);
                TestJSContextDeathModules jsModules(m_Context);

                info->SetPath(appRoot / std::filesystem::path("js/NotExists.js"));
                JSModuleInfoSharedPtr module = jsModules.TestLoadModuleTree(m_Context, info);
                std::exit(0);
            },
                         "");
        }

    }
}
