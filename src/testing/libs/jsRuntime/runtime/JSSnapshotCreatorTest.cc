// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"

#include "JSSnapshotCreator.h"
#include "CppBridge/V8FunctionTemplate.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSnapshotCreatorTest = V8Fixture;

        static std::string g_FunctionString;
        void testFunctionInRuntime(std::string inString)
        {
            g_FunctionString = inString;
        }

        TEST_F(JSnapshotCreatorTest, Playground)
        {
            std::filesystem::path snapshotFile = "playground.dat";
            snapshotFile = s_TestDir / snapshotFile;

            m_App->SetEntryPointScript("%JS%/LoadModules.js");

            JSAppSharedPtr snapApp = m_App->CreateSnapshotApp();
            snapApp->AppInit();
            JSRuntimeSharedPtr runtime = snapApp->GetJSRuntime();

            v8::Local<v8::FunctionTemplate> funcTemplate = CppBridge::CreateFunctionTemplate(
                runtime->GetIsolate(), Utils::MakeCallback(testFunctionInRuntime));
            
            JSContextSharedPtr jsContext = snapApp->CreateJSContext("default");

            JSSnapshotCreator creator(snapApp);

            EXPECT_TRUE(creator.CreateSnapshotFile(snapshotFile));
            snapApp->DisposeApp();

            JSSnapshotProviderSharedPtr playgroundSnap = std::make_shared<JSSnapshotProvider>(snapshotFile);
            JSAppSharedPtr restore = std::make_shared<JSApp>("Restored", playgroundSnap);
            restore->Initialize(s_TestDir, false, std::make_shared<JSContextCreator>());

            runtime = restore->GetJSRuntime();
            JSContextSharedPtr jsContext = restore->CreateJSContext("Restored");

            restore->DisposeApp();
        }
    }
}