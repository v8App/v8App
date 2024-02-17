
// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8Fixture.h"
#include "ScriptStartupDataManager.h"

namespace v8App
{
    namespace JSRuntime
    {
        using ScriptStartupDataManTest = V8Fixture;

        TEST(V8TestStartupICUData, TestV8StartupInitlaization)
        {
            std::string error;
            Runfiles *runFiles = Runfiles::CreateForTest(&error);
            ASSERT_EQ(0, error.length());
            ASSERT_NE(nullptr, runFiles);

            std::string icuData = runFiles->Rlocation("v8App/third_party/v8/*/icudtl.dat");
            std::string snapshotData = runFiles->Rlocation("v8App/third_party/v8/*/snapshot_blob.bin");

            ASSERT_FALSE(ScriptStartupDataManager::InitializeICUData(""));
            ASSERT_FALSE(ScriptStartupDataManager::InitializeICUData("notExist"));
            ASSERT_FALSE(ScriptStartupDataManager::InitializeStartupData(""));
#ifdef V8APP_DEBUG
            //this test can only be done in ebug since in release builds the data will be baked in
            ASSERT_FALSE(ScriptStartupDataManager::InitializeStartupData("notExist"));
#endif

            ASSERT_TRUE(ScriptStartupDataManager::InitializeICUData(icuData));
            ASSERT_TRUE(ScriptStartupDataManager::InitializeStartupData(snapshotData));
        }
    }
}