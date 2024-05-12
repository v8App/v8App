// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test_main.h"
#include "V8Fixture.h"

#include "JSSnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        using JSnapshotCreatorTest = V8Fixture;

        TEST_F(JSnapshotCreatorTest, Playground)
        {
            std::filesystem::path snapshotFile = "playground.dat";
            snapshotFile = s_TestDir / snapshotFile;

            m_App->SetEntryPointScript("%JS%/LoadModules.js");

            JSAppSharedPtr snapApp = m_App->CreateSnapshotApp();
            snapApp->AppInit();

            JSSnapshotCreator creator(snapApp);

            EXPECT_TRUE(creator.CreateSnapshotFile(snapshotFile));
            snapApp->DisposeApp();
        }
    }
}