// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifdef USE_JSRUNTIME

#include "V8Fixture.h"
#include "test_main.h"
#include "TestSnapshotProvider.h"

#include "Utils/Environment.h"

#include "JSApp.h"
#include "JSContext.h"
#include "V8ContextProvider.h"
#include "V8RuntimeProvider.h"

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
            m_Providers.m_SnapshotProvider = std::make_shared<TestSnapshotProvider>();
            m_Providers.m_ContextProvider = std::make_shared<V8ContextProvider>();
            m_Providers.m_RuntimeProvider = std::make_shared<V8RuntimeProvider>();

            m_App = std::make_shared<JSApp>();
            m_App->Initialize(suiteName, s_TestDir, m_Providers, false);
            
            m_Runtime = m_App->GetMainRuntime();
            ASSERT_NE(nullptr, m_Runtime);

            m_Isolate = m_Runtime->GetIsolate();
            ASSERT_NE(m_Isolate, nullptr);
            m_Context = m_Runtime->CreateContext(suiteName, "");
        }

        void V8Fixture::TearDown()
        {
            if (m_Context != nullptr && m_Runtime != nullptr)
            {
                V8IsolateScope isolateScope(m_Runtime->GetIsolate());
                V8HandleScope scope(m_Runtime->GetIsolate());
                m_Runtime->DisposeContext(m_Context);
            }
            m_Isolate = nullptr;
            m_Context.reset();
            m_Runtime.reset();
            m_App->DisposeApp();
            m_App.reset();
        }
    } // namespace JSRuntime
} // namespace v8App

#endif
