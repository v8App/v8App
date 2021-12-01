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
        V8TestFixture::V8TestFixture()
        {
        }

        V8TestFixture::~V8TestFixture() = default;

        void V8TestFixture::SetUp()
        {
            std::string error;
            m_RunFiles.reset(Runfiles::CreateForTest(&error));

            if (error != "")
            {
                std::cout << error << std::endl;
                ASSERT_TRUE(false);
            }
            ASSERT_NE(nullptr, m_RunFiles.get());

            //setup the global test log
            TestUtils::TestLogSink* testSink = TestUtils::TestLogSink::GetGlobalSink();
            testSink->FlushMessages();

            std::string icuData = m_RunFiles->Rlocation("com_github_v8app_v8app/third_party/v8/*/icudtl.dat");
            std::string snapshotData = m_RunFiles->Rlocation("com_github_v8app_v8app/third_party/v8/*/snapshot_blob.bin");

            ASSERT_NE("", icuData);
            ASSERT_NE("", snapshotData);

            ASSERT_TRUE(ScriptStartupDataManager::InitializeICUData(icuData));
            ASSERT_TRUE(ScriptStartupDataManager::InitializeStartupData(snapshotData));

            V8Platform::InitializeV8();
            m_Runtime = std::make_unique<JSRuntime>(IdleTasksSupport::kIdleTasksEnabled);

            IsolateWeakPtr isolate = m_Runtime->GetIsolate();
            ASSERT_FALSE(isolate.expired());
            m_Isolate = isolate.lock().get();
            m_Isolate->Enter();
            v8::HandleScope scope(m_Isolate);
            m_Context = m_Runtime->CreateContext();
            v8::Local<v8::Context> context = m_Context.lock().get()->GetContext();
            ASSERT_FALSE(context.IsEmpty());

            context->Enter();
            //celar out the module search paths so the tests can start fresh
            JSModules::RemoveAllRootPaths();
        }

        void V8TestFixture::TearDown()
        {
            if (m_Isolate == nullptr)
            {
                return;
            }
            {
                v8::HandleScope scope(m_Isolate);
                m_Context.lock().get()->GetContext()->Exit();
            }
            m_Isolate->Exit();
            m_Isolate = nullptr;
            m_Runtime.reset();
        }

    } // namespace JSRuntime
} // namespace v8App