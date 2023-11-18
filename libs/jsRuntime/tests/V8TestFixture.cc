// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "TestLogSink.h"
#include "V8TestFixture.h"
#include "ScriptStartupDataManager.h"

namespace v8App
{
    namespace JSRuntime
    {
        std::unique_ptr<Runfiles> InitV8Platform()
        {
            std::string icu_name = Utils::GetEnvironmentVar("V8_ICU_DATA");
            std::string snapshot_name = Utils::GetEnvironmentVar("V8_SNAPSHOT_BIN");

            if(icu_name.empty() || snapshot_name.empty())
            {
                EXPECT_TRUE(false) << "Failed to find one or both of env vars V8_ICU_DATA, V8_SNAPSHOT_BIN";
            }

            std::string error;
            std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
            if(error.empty() == false)
            {
                EXPECT_TRUE(false) << error;
            }
            std::string icuData = runfiles->Rlocation(icu_name);
            std::string snapshotData = runfiles->Rlocation(snapshot_name);

            EXPECT_NE("", icuData);
            EXPECT_NE("", snapshotData);

            EXPECT_TRUE(ScriptStartupDataManager::InitializeICUData(icuData));
            EXPECT_TRUE(ScriptStartupDataManager::InitializeStartupData(snapshotData));

            V8Platform::InitializeV8();
            return runfiles;
        }

        V8TestFixture::V8TestFixture()
        {
        }

        V8TestFixture::~V8TestFixture() = default;

        void V8TestFixture::SetUp()
        {
            // setup the global test log
            TestUtils::TestLogSink *testSink = TestUtils::TestLogSink::GetGlobalSink();
            testSink->FlushMessages();
            m_RunFiles = InitV8Platform();
            EXPECT_NE(nullptr, m_RunFiles);
           
            m_Runtime = std::make_unique<JSRuntime>(IdleTasksSupport::kIdleTasksEnabled);

            IsolateWeakPtr isolate = m_Runtime->GetIsolate();
            ASSERT_FALSE(isolate.expired());
            m_Isolate = isolate.lock().get();
            m_Isolate->Enter();
            m_Context = m_Runtime->CreateContext("test").lock();
        }

        void V8TestFixture::TearDown()
        {
            if (m_Isolate == nullptr)
            {
                return;
            }
            {
                v8::HandleScope scope(m_Isolate);
                m_Context->GetContext()->Exit();
            }
            m_Isolate->Exit();
            m_Isolate = nullptr;
            m_Runtime.reset();
        }

        v8::Local<v8::Context> V8TestFixture::GetContextAndEnter()
        {
            v8::EscapableHandleScope scope(m_Isolate);
            v8::Local<v8::Context> context = m_Context->GetContext();
            context->Enter();
            return scope.Escape(context);
        }

    } // namespace JSRuntime
} // namespace v8App