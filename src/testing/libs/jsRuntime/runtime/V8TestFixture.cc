// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8TestFixture.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        void V8TestFixture::SetUp()
        {
            // setup the global test log
            TestUtils::TestLogSink *testSink = TestUtils::TestLogSink::GetGlobalSink();
            testSink->FlushMessages();

            if (s_InitSingleton == nullptr)
            {
                s_InitSingleton = std::make_unique<InitializeV8Testing>();
            }

            std::string test_dir = Utils::GetEnvironmentVar("TEST_SRCDIR");
            EXPECT_NE("", test_dir);

            m_App = std::make_shared<JSApp>("test");
            m_App->Initialize();
            m_Runtime = m_App->CreateJSRuntime("libJSRuntimeTest");
            m_Runtime->SetContextCreationHelper(std::make_unique<JSContextCreator>());

            m_Isolate = m_Runtime->GetIsolate().get();
            ASSERT_NE(m_Isolate, nullptr);
            m_Context = m_Runtime->CreateContext("libJSRuntimeTest").lock();
            //MANIFEST has the path to the original ones we want the copies under-bazel-bin
            std::filesystem::path testFiles = std::filesystem::path(test_dir) / std::filesystem::path("v8App/libs/jsRuntime/tests/test-files/app-root");
            m_App->GetAppRoots()->SetAppRootPath(testFiles);
        }

        void V8TestFixture::TearDown()
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

        v8::Local<v8::Context> V8TestFixture::GetContextAndEnter()
        {
            v8::EscapableHandleScope scope(m_Isolate);
            v8::Local<v8::Context> context = m_Context->GetLocalContext();
            context->Enter();
            return scope.Escape(context);
        }

    } // namespace JSRuntime
} // namespace v8App