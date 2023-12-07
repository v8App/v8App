// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tools/cpp/runfiles/runfiles.h"

#include "TestLogSink.h"
#include "Utils/Environment.h"

#include "JSRuntime.h"
#include "V8Platform.h"

using bazel::tools::cpp::runfiles::Runfiles;

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
            explicit TestJSRuntime(IdleTasksSupport inEnableIdle);
            void ClearIsolate() { m_Isolate.reset(); }
        };

        class JSRuntimeDeathFixture : public testing::Test
        {
        public:
            JSRuntimeDeathFixture() {}
            ~JSRuntimeDeathFixture() {}

            virtual void SetUp() override
            {
                std::string icu_name = Utils::GetEnvironmentVar("V8_ICU_DATA");
                std::string snapshot_name = Utils::GetEnvironmentVar("V8_SNAPSHOT_BIN");

                if (icu_name.empty() || snapshot_name.empty())
                {
                    EXPECT_TRUE(false) << "Failed to find one or both of env vars V8_ICU_DATA, V8_SNAPSHOT_BIN";
                }

                std::string error;
                std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
                if (error.empty() == false)
                {
                    EXPECT_TRUE(false) << error;
                }
                std::string icuData = runfiles->Rlocation(icu_name);
                std::string snapshotData = runfiles->Rlocation(snapshot_name);

                EXPECT_NE("", icuData);
                EXPECT_NE("", snapshotData);

                v8::V8::InitializeICU(icuData.c_str());
                v8::V8::InitializeExternalStartupDataFromFile(snapshotData.c_str());
                std::unique_ptr<JSRuntimeIsolateHelper> helper = std::make_unique<JSRuntimeIsolateHelper>();
                V8Platform::InitializeV8(std::move(helper));
            }
            virtual void TearDown() override
            {
                V8Platform::ShutdownV8();
            }
        };

        TEST_F(JSRuntimeDeathFixture, SetObjectTemplate)
        {
            ASSERT_DEATH({
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(IdleTasksSupport::kIdleTasksEnabled);
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());

                runtimePtr->ClearIsolate();

                v8::Local<v8::ObjectTemplate> objTemplate;
                struct TemplateInfo info;
                runtimePtr->SetObjectTemplate(&info, objTemplate);
                std::exit(0);
            },
                         "");
        }

        TEST_F(JSRuntimeDeathFixture, GetObjectTemplate)
        {
            ASSERT_DEATH({
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(IdleTasksSupport::kIdleTasksEnabled);
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());
                v8::Local<v8::ObjectTemplate> objTemplate;

                runtimePtr->ClearIsolate();

                struct TemplateInfo info;
                runtimePtr->GetObjectTemplate(&info);
                std::exit(0);
            },
                         "");
        }

        TEST_F(JSRuntimeDeathFixture, SetFunctionTemplate)
        {
            ASSERT_DEATH({
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(IdleTasksSupport::kIdleTasksEnabled);
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::HandleScope scope(runtimePtr->GetIsolate().get());
                v8::Local<v8::FunctionTemplate> funcTemplate;

                runtimePtr->ClearIsolate();

                struct TemplateInfo info;
                runtimePtr->SetFunctionTemplate(&info, funcTemplate);
                std::exit(0);
            },
                         "");
        }

        TEST_F(JSRuntimeDeathFixture, GetFunctionTemplate)
        {
            ASSERT_DEATH({
                std::shared_ptr<TestJSRuntime> runtimePtr = std::make_shared<TestJSRuntime>(IdleTasksSupport::kIdleTasksEnabled);
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

        TEST_F(JSRuntimeDeathFixture, CreateContext)
        {
            ASSERT_DEATH({
                JSRuntimeSharedPtr runtimePtr = JSRuntime::CreateJSRuntime(IdleTasksSupport::kIdleTasksEnabled);
                v8::Isolate::Scope isolateScope(runtimePtr->GetIsolate().get());
                v8::Local<v8::ObjectTemplate> objTemplate;
                runtimePtr->CreateContext("test");
                std::exit(0);
            },
                         "");
        }
    }
}