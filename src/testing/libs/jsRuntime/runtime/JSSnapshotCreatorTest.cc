// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"

#include "JSSnapshotCreator.h"
#include "V8ExternalRegistry.h"
#include "JSUtilities.h"
#include "CppBridge/V8FunctionTemplate.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSnapshotCreatorTest = V8Fixture;

        static std::string g_FunctionString;
        static std::string g_FunctionString2;
        void testFunctionInRuntime(std::string inString)
        {
            g_FunctionString = inString;
        }

        void testFunctionInRuntime2(std::string inString)
        {
            g_FunctionString2 = inString;
        }

        void RegisterFuncTemplate(V8Isolate* inIsolate, v8::Local<v8::ObjectTemplate>&inGlobal)
        {
            auto funcTpl = CppBridge::CreateFunctionTemplate(inIsolate, Utils::MakeCallback(testFunctionInRuntime));
            inGlobal->Set(JSUtilities::StringToV8(inIsolate, "test"), funcTpl);
            funcTpl = CppBridge::CreateFunctionTemplate(inIsolate, Utils::MakeCallback(testFunctionInRuntime2));
            inGlobal->Set(JSUtilities::StringToV8(inIsolate, "test2"), funcTpl);
        }

        REGISTER_FUNCS(testFunctionInRuntime)
        {
            V8ExternalRegistry::Register(Utils::MakeCallback(testFunctionInRuntime));
            V8ExternalRegistry::RegisterGlobalRegisterer(&RegisterFuncTemplate);
        }

        TEST_F(JSnapshotCreatorTest, Playground)
        {
            std::filesystem::path snapshotFile = "playground.dat";
            snapshotFile = s_TestDir / snapshotFile;

            m_App->SetEntryPointScript("%JS%/LoadModules.js");

            JSAppSharedPtr snapApp = m_App->CreateSnapshotApp();
            snapApp->AppInit();
            JSRuntimeSharedPtr runtime = snapApp->GetJSRuntime();
            runtime->CreateGlobalTemplate(true);

            {
                v8::Isolate *isolate = snapApp->GetJSRuntime()->GetIsolate();
                v8::Isolate::Scope iScope(isolate);
                v8::HandleScope hScope(isolate);

                JSContextSharedPtr jsContext = snapApp->CreateJSContext("default");
                V8LocalContext context = jsContext->GetLocalContext();
                v8::Context::Scope cScope(context);

                const char csource1[] = R"(
                    test('test');
                    test2('test2');
                )";

                v8::TryCatch try_catch(isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(context).ToLocal(&result) == false)
                {
                    std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                    EXPECT_TRUE(false);
                }

                EXPECT_EQ("test", g_FunctionString);
                EXPECT_EQ("test2", g_FunctionString2);
            }

            JSSnapshotCreator creator(snapApp);

            EXPECT_TRUE(creator.CreateSnapshotFile(snapshotFile));
            snapApp->DisposeApp();

            JSSnapshotProviderSharedPtr playgroundSnap = std::make_shared<JSSnapshotProvider>(snapshotFile);
            JSAppSharedPtr restore = std::make_shared<JSApp>("Restored", playgroundSnap);
            restore->Initialize(s_TestDir, false, std::make_shared<JSContextCreator>());

            runtime = restore->GetJSRuntime();
            runtime->CreateGlobalTemplate(false);
            JSContextSharedPtr jsContext = restore->CreateJSContext("Restored");
            {
                v8::Isolate *isolate = runtime->GetIsolate();
                v8::Isolate::Scope iScope(isolate);
                v8::HandleScope hScope(isolate);
                V8LocalContext context = jsContext->GetLocalContext();
                v8::Context::Scope cScope(context);

                const char csource1[] = R"(
                    test('test2');
                )";

                v8::TryCatch try_catch(isolate);

                v8::Local<v8::String> source1 = JSUtilities::StringToV8(isolate, csource1);

                v8::Local<v8::Script> script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                v8::Local<v8::Value> result;
                if (script1->Run(context).ToLocal(&result) == false)
                {
                    std::cout << "Script Error: " << JSUtilities::GetStackTrace(context, try_catch) << std::endl;
                    EXPECT_TRUE(false);
                }

                EXPECT_EQ("test", g_FunctionString);
                EXPECT_EQ("test2", g_FunctionString2);
            }
            restore->DisposeApp();
        }
    }
}