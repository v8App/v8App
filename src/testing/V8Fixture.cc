// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifdef USE_JSRUNTIME

#include "V8Fixture.h"

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

            m_App = std::make_shared<JSApp>("test");
            m_App->Initialize();
            const char* suiteName = ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name();
            m_Runtime = m_App->CreateJSRuntime(suiteName);
            m_Runtime->SetContextCreationHelper(std::make_unique<JSContextCreator>());

            m_Isolate = m_Runtime->GetIsolate().get();
            ASSERT_NE(m_Isolate, nullptr);
            m_Context = m_Runtime->CreateContext(suiteName).lock();
            m_App->GetAppRoots()->SetAppRootPath(s_TestDir);
        }

        void V8Fixture::TearDown()
        {
            if (m_Isolate == nullptr)
            {
                return;
            }
            {
                v8::Isolate::Scope isolateScope(m_Isolate);
                v8::HandleScope scope(m_Isolate);
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
