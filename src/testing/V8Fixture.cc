// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifdef USE_JSRUNTIME

#include "V8Fixture.h"
#include "test_main.h"
#include "TestSnapshotProvider.h"

#include "Utils/Environment.h"
#include "JSContext.h"

namespace v8App 
{
    namespace JSRuntime
    {
        void V8Fixture::SetUp()
        {
            // setup the global test log
            TestUtils::TestLogSink *testSink = TestUtils::TestLogSink::GetGlobalSink();
            testSink->FlushMessages();

            const char* suiteName = ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name();
            std::shared_ptr<TestSnapshotProvider> snapProvider = std::make_shared<TestSnapshotProvider>();
            m_App = std::make_shared<JSApp>(suiteName, snapProvider);
            //no need for a parth the test prover doesn't do the loading the main function does
            m_App->InitializeApp(s_TestDir);
            
            m_Runtime = m_App->GetJSRuntime();
            ASSERT_NE(nullptr, m_Runtime);

            m_Runtime->SetContextCreationHelper(std::make_unique<JSContextCreator>());

            m_Isolate = m_Runtime->GetIsolate();
            ASSERT_NE(m_Isolate, nullptr);
            m_Context = m_Runtime->CreateContext(suiteName);
        }

        void V8Fixture::TearDown()
        {
            if (m_Runtime != nullptr)
            {
                v8::Isolate::Scope isolateScope(m_Runtime->GetIsolate());
                v8::HandleScope scope(m_Runtime->GetIsolate());
                m_Runtime->DisposeContext(m_Context);
            }
            m_Isolate = nullptr;
            m_Runtime.reset();
            m_App->DisposeApp();
            m_App.reset();
        }
    } // namespace JSRuntime
} // namespace v8App

#endif
