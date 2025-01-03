
// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "V8Fixture.h"
#include "TestFiles.h"

#include "Utils/Format.h"

#include "JSApp.h"
#include "JSRuntime.h"
#include "V8AppSnapshotCreator.h"
#include "V8AppSnapshotProvider.h"

namespace v8App
{
    namespace JSRuntime
    {
        using V8AppSnapshotRestoreTest = V8Fixture;

        TEST_F(V8AppSnapshotRestoreTest, RestoreSnapshot)
        {
            std::filesystem::path testRoot = s_TestDir / "RestoreSnapshot";
            EXPECT_TRUE(TestUtils::CreateAppDirectory(testRoot));

            std::filesystem::path snapshotFile = "RestoreSnapshot.dat";
            snapshotFile = testRoot / snapshotFile;

            JSAppSharedPtr snapApp = m_App->CloneAppForSnapshotting();
            {
                JSRuntimeSharedPtr secondRuntime = snapApp->CreateJSRuntimeFromIndex("SecondRuntime", 0,
                                                                                     JSRuntimeSnapshotAttributes::SnapshotAndRestore);
                JSRuntimeSharedPtr thirdRuntime = snapApp->CreateJSRuntimeFromIndex("ThirdRuntime", 0,
                                                                                    JSRuntimeSnapshotAttributes::SnapshotOnly);
                {
                    // V8Isolate::Scope iScope(secondRuntime->GetIsolate());
                    // V8HandleScope hScope(secondRuntime->GetIsolate());

                    JSContextSharedPtr jsContext = secondRuntime->CreateContext("SecondContext", "");
                    if (jsContext == nullptr)
                    {
                        // so the snap app is disposed of or we'll error on test shutdown
                        snapApp->DisposeApp();
                        ASSERT_FALSE(true);
                    }
                }
            }
            V8AppSnapshotCreator creator;

            // nno need to create a context since the runtime always creates a bare v8 context as default
            if (creator.CreateSnapshot(*snapApp, snapshotFile) == false)
            {
                // so the snap app is disposed of or we'll error on test shutdown
                snapApp->DisposeApp();
                ASSERT_FALSE(true);
            }
            // snapApp->DisposeRuntime(thirdRuntime);
            snapApp->DisposeApp();

            V8AppSnapshotProviderSharedPtr snapProvider = std::make_shared<V8AppSnapshotProvider>();
            ASSERT_TRUE(snapProvider->LoadSnapshotData(snapshotFile));

            AppProviders providers = m_Providers;
            providers.m_SnapshotProvider = snapProvider;
            JSAppSharedPtr restore = snapProvider->RestoreApp(testRoot, providers);
            if (restore == nullptr)
            {
                ASSERT_FALSE(true);
            }
            {
                JSRuntimeSharedPtr secondRuntime = restore->GetRuntimeByName("SecondRuntime");
                if (secondRuntime == nullptr)
                {
                    restore->DisposeApp();
                    ASSERT_FALSE(true);
                }

                JSContextSharedPtr context = secondRuntime->GetContextByName("SecondContext");
                if (context == nullptr)
                {
                    restore->DisposeApp();
                    ASSERT_FALSE(true);
                }
            }
            restore->DisposeApp();
            EXPECT_EQ(nullptr, snapProvider->RestoreApp(testRoot, providers));
        }
    }
}
