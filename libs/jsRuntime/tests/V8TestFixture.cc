// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "V8TestFixture.h"
#include "third_party/bazel-runfiles/runfiles_src.h"

using bazel::tools::cpp::runfiles::Runfiles;

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
            std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
            if (error != "")
            {
                std::cout << error << std::endl;
                ASSERT_TRUE(false);
            }
            ASSERT_NE(nullptr, runfiles);

            std::string icuData = runfiles->Rlocation("com_github_v8app_v8app/third_party/v8/*/icudtl.dat");
            std::string snapshotData = runfiles->Rlocation("com_github_v8app_v8app/third_party/v8/*/snapshot_blob.bin");

            ASSERT_NE("", icuData);
            ASSERT_NE("", snapshotData);

            //First test the innit of v8
            v8::V8::InitializeICU(icuData.c_str());
            v8::V8::InitializeExternalStartupDataFromFile(snapshotData.c_str());
            V8Platform::InitializeV8();
            m_Runtime = std::make_unique<JSRuntime>(IdleTasksSupport::kIdleTasksEnabled);

            IsolateWeakPtr isolate = m_Runtime->GetIsolate();
            ASSERT_FALSE(isolate.expired());
            m_Isolate = isolate.lock().get();
            m_Isolate->Enter();
            v8::HandleScope scope(m_Isolate);
            v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_Isolate);
            m_Context.Reset(m_Isolate, v8::Context::New(m_Isolate, nullptr, global));
            m_Context.Get(m_Isolate)->Enter();
            //            v8::Local<v8::Context>::New(m_Isolate, m_Context->Enter();
        }

        void V8TestFixture::TearDown()
        {
            if (m_Isolate == nullptr)
            {
                return;
            }
            {
                v8::HandleScope scope(m_Isolate);
                v8::Local<v8::Context>::New(m_Isolate, m_Context)->Exit();
                m_Context.Reset();
            }
            m_Isolate->Exit();
            m_Isolate = nullptr;
            m_Runtime.reset();
        }

    } // namespace JSRuntime
} // namespace v8App