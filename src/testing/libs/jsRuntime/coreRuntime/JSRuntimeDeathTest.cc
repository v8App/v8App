// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TestLogSink.h"
#include "Utils/Environment.h"

#include "JSRuntime.h"
#include "V8Platform.h"

#include "V8InitApp.h"

namespace v8App
{
    namespace JSRuntime
    {        
        struct TemplateInfo
        {
            bool m_Bool;
        };

        class TestJSRuntime : public JSRuntime
        {
        public:
            explicit TestJSRuntime(JSAppSharedPtr inApp,  IdleTasksSupport inEnableIdle, std::string inName) : JSRuntime(inApp, inEnableIdle, inName) {}
            void ClearIsolate() { m_Isolate.reset(); }
        };

 
        TEST(JSRuntimeDeathTest, SetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                //V8PlatformInitFixture::SetUpTestSuite();
                JSAppSharedPtr app = std::make_shared<JSApp>("test");
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "SetObjectTemplate");
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());

                runtimePtr->ClearIsolate();

                v8::Local<v8::ObjectTemplate> objTemplate;
                struct TemplateInfo info;
                runtimePtr->SetObjectTemplate(&info, objTemplate);
                //V8PlatformInitFixture::TearDownTestSuite();
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, GetObjectTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                //V8PlatformInitFixture::SetUpTestSuite();
                JSAppSharedPtr app = std::make_shared<JSApp>("test");
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetObjectTemplate");
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());
                v8::Local<v8::ObjectTemplate> objTemplate;

                runtimePtr->ClearIsolate();

                struct TemplateInfo info;
                runtimePtr->GetObjectTemplate(&info);
                //V8PlatformInitFixture::TearDownTestSuite();
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, SetFunctionTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                //V8PlatformInitFixture::SetUpTestSuite();
                JSAppSharedPtr app = std::make_shared<JSApp>("test");
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "SetFunctionTemplate");
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());
                v8::Local<v8::FunctionTemplate> funcTemplate;

                runtimePtr->ClearIsolate();

                struct TemplateInfo info;
                runtimePtr->SetFunctionTemplate(&info, funcTemplate);
                //V8PlatformInitFixture::TearDownTestSuite();
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, GetFunctionTemplate)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSAppSharedPtr app = std::make_shared<JSApp>("test");
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(app, IdleTasksSupport::kIdleTasksEnabled, "GetFunctionTemplate");
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());
                v8::Local<v8::FunctionTemplate> funcTemplate;

                runtimePtr->ClearIsolate();

                struct TemplateInfo info;
                runtimePtr->GetFunctionTemplate(&info);
                std::exit(0);
            },
                         "");
        }

        TEST(JSRuntimeDeathTest, CreateContext)
        {
            GTEST_FLAG_SET(death_test_style, "threadsafe");
            ASSERT_DEATH({
                JSAppSharedPtr app = std::make_shared<JSApp>("test");
                JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(app, IdleTasksSupport::kIdleTasksEnabled, "CreateContext");
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::Local<v8::ObjectTemplate> objTemplate;
                runtimePtr->CreateContext("test");
                std::exit(0);
            },
                         "");
        }
    }
}