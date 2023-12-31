// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "tools/cpp/runfiles/runfiles.h"

#include "Utils/Environment.h"

#include "V8Platform.h"
#include "JSRuntime.h"
#include "JSContext.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace v8App
{
    namespace JSRuntime
    {
        // Since we can only init/tear down V8 once per process we need a singleton class
        // inits on constrcution and tears it down at desctruction.
        // Sicne Death Test run first and in a forked process they can tear it down with out
        // issues for the main process.
        class InitializeV8Testing
        {
        public:
            InitializeV8Testing()
            {
                std::string icu_name = Utils::GetEnvironmentVar("V8_ICU_DATA");
                std::string snapshot_name = Utils::GetEnvironmentVar("V8_SNAPSHOT_BIN");

                if (icu_name.empty() || snapshot_name.empty())
                {
                    EXPECT_TRUE(false) << "Failed to find one or both of env vars V8_ICU_DATA, V8_SNAPSHOT_BIN";
                }

                std::string error;
                m_Runfiles.reset(Runfiles::CreateForTest(&error));
                if (error.empty() == false)
                {
                    EXPECT_TRUE(false) << error;
                }
                std::string icuData = m_Runfiles->Rlocation(icu_name);
                std::string snapshotData = m_Runfiles->Rlocation(snapshot_name);

                EXPECT_NE("", icuData);
                EXPECT_NE("", snapshotData);

                v8::V8::InitializeICU(icuData.c_str());
                v8::V8::InitializeExternalStartupDataFromFile(snapshotData.c_str());

                PlatformIsolateHelperUniquePtr helper = std::make_unique<JSRuntimeIsolateHelper>();
                V8Platform::InitializeV8(std::move(helper));
                // used to prevent the static from being released before this static is destroyed.
                m_Holder = V8Platform::Get();
            }

            ~InitializeV8Testing()
            {
                V8Platform::ShutdownV8();
                m_Holder.reset();
            }

            std::unique_ptr<Runfiles> m_Runfiles;

        private:
            std::shared_ptr<V8Platform> m_Holder;
        };

        // process singleton for v8 Init/Tear down
        static std::unique_ptr<InitializeV8Testing> s_InitSingleton;

        class V8PlatformInitFixture : public testing::Test
        {
        public:
            V8PlatformInitFixture()
            {
                if (s_InitSingleton == nullptr)
                {
                    s_InitSingleton = std::make_unique<InitializeV8Testing>();
                }
                m_App = std::make_shared<JSApp>("testCore");
                m_App->Initialize();
            }
            ~V8PlatformInitFixture() {
                m_App->DisposeApp();
                m_App.reset();
            }

            JSAppSharedPtr m_App;
        };

    }
}