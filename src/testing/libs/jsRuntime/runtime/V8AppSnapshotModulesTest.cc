// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"
#include "TestFiles.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8AppSnapshotCreator.h"
#include "V8AppSnapshotProvider.h"
#include "CppBridge/CallbackRegistry.h"
#include "JSUtilities.h"
#include "CppBridge/V8FunctionTemplate.h"
#include "CppBridge/V8CppObject.h"
#include "CppBridge/V8ObjectTemplateBuilder.h"
#include "CppBridge/V8CppObjHandle.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8AppSnapshotModulesTest = V8Fixture;

        TEST_F(V8AppSnapshotModulesTest, ModuleSnapTest)
        {
            std::filesystem::path snapshotFile = "SnapshotModulesTest.dat";
            std::filesystem::path testRoot = s_TestDir / "SnapshotModulesTest";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::filesystem::path moduleFile = s_TestDir / "js/snapModules/main.js";

            snapshotFile = testRoot / snapshotFile;

            JSAppSharedPtr snapApp = m_App->CloneAppForSnapshotting();
            JSRuntimeSharedPtr runtime = snapApp->GetMainRuntime();
            // remove the test fixtures auto created one that got cloned
            runtime->DisposeContext(m_Context->GetName());
            bool failedTest = false;
            {
                V8Isolate *isolate = runtime->GetIsolate();
                V8IsolateScope iScope(isolate);
                V8HandleScope hScope(isolate);
                V8TryCatch try_catch(isolate);

                JSContextSharedPtr jsContext = runtime->CreateContext("default", "");
                V8LContext context = jsContext->GetLocalContext();
                V8ContextScope cScope(context);

                V8LValue value = jsContext->RunModule(moduleFile);
                if (try_catch.HasCaught())
                {
                    failedTest = true;
                }
            }
            // need to dispose after the scopes are released
            if (failedTest)
            {
                std::cout << "Failed to run the module" << std::endl;
                snapApp->DisposeApp();
                ASSERT_FALSE(true);
            }

            IJSSnapshotCreatorSharedPtr creator = std::make_shared<V8AppSnapshotCreator>();
            snapApp->SetSnapshotCreator(creator);

            if (creator->CreateSnapshot(*snapApp, snapshotFile) == false)
            {
                snapApp->DisposeApp();
                ASSERT_FALSE(true);
            }
            snapApp->DisposeApp();

            V8AppSnapshotProviderSharedPtr snapProvider = std::make_shared<V8AppSnapshotProvider>();
            ASSERT_TRUE(snapProvider->LoadSnapshotData(snapshotFile));

            AppProviders providers = m_Providers;
            providers.m_SnapshotProvider = snapProvider;

            JSAppSharedPtr restore = std::make_shared<JSApp>();
            ASSERT_TRUE(restore->Initialize("SnapshotObjectsTest", testRoot, providers));

            {
                runtime = restore->GetMainRuntime();
                V8Isolate *isolate = runtime->GetIsolate();
                isolate->Enter();
                V8HandleScope hScope(isolate);
                {
                    JSContextSharedPtr jsContext = runtime->CreateContextFromSnapshot("default", 0);
                    if (jsContext == nullptr)
                    {
                        isolate->Exit();
                        restore->DisposeApp();
                        ASSERT_FALSE(true);
                    }
                    V8LContext context = jsContext->GetLocalContext();
                    V8ContextScope cScope(context);

                    // TODO: Figure out a way to test that the module was restored
                    // Since this doesn't work
                    /**
                                        const char csource1[] = R"(
                                                            module1.function2();
                                                        )";

                                        V8TryCatch try_catch(isolate);

                                        V8LString source1 = JSUtilities::StringToV8(isolate, csource1);

                                        V8LScript script1 = v8::Script::Compile(context, source1).ToLocalChecked();

                                        V8LValue result;
                                        if (script1->Run(context).ToLocal(&result) == false)
                                        {
                                            std::cout << "Script Error: " << JSUtilities::GetStackTrace(isolate, try_catch) << std::endl;
                                            EXPECT_TRUE(false);
                                        }
                                        */
                }
                isolate->Exit();
            }
            {
                runtime = restore->GetMainRuntime();
                V8Isolate *isolate = runtime->GetIsolate();
                isolate->Enter();
                V8HandleScope hScope(isolate);
                {
                    JSContextSharedPtr jsContext = runtime->CreateContextFromSnapshot("default2", 0);
                    if (jsContext == nullptr)
                    {
                        isolate->Exit();
                        restore->DisposeApp();
                        ASSERT_FALSE(true);
                    }
                }
                isolate->Exit();
            }
            restore->DisposeApp();
        }
    }
}
