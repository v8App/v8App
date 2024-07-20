// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Utils/Format.h"

#include "JSSnapshotCreator.h"
#include "JSContext.h"
#include "CppBridge/V8CppObject.h"
#include "CppBridge/CallbackRegistry.h"
#include "V8AppPlatform.h"

namespace v8App
{
    namespace JSRuntime
    {

        JSSnapshotCreator::JSSnapshotCreator(JSAppSharedPtr inApp) : m_App(inApp)
        {
        }

        JSSnapshotCreator::~JSSnapshotCreator()
        {
        }

        bool JSSnapshotCreator::CreateSnapshotFile(std::filesystem::path inSnapshotFile)
        {
            IJSSnapshotProvider creator = m_App->GetSnapshotProvider();
            if (creator == nullptr)
            {
                return false;
            }

            JSRuntimeSharedPtr runtime  = m_App->GetMainRuntime();
            V8Isolate *isolate = runtime->GetIsolate();

            JSContextSharedPtr defaultContext = runtime->CreateContext("v8-default", "");
            // save set the default context which is the normal v8 one
            {
                V8IsolateScope iScope(isolate);
                V8HandleScope hScope(isolate);

                V8LContext lContext = defaultContext->GetLocalContext();
                V8ContextScope cScope(lContext);

                creator->SetDefaultContext(lContext);
                runtime->CloseOpenHandlesForSnapshot();
            }

            // now go through the contexts that have been created and add them
            {
                

            }

            {
                // V8AppPlatform::Get()->SetWorkersPaused(true);
                V8StartupData data = creator->CreateBlob(V8SnapCreator::FunctionCodeHandling::kClear);
                if (data.raw_size == 0)
                {
                    V8AppPlatform::Get()->SetWorkersPaused(false);
                    return false;
                }
                // V8AppPlatform::Get()->SetWorkersPaused(false);

                std::ofstream snapFile(inSnapshotFile, std::ios::out | std::ios::binary);
                if (snapFile.is_open() == false || snapFile.fail())
                {
                    return false;
                }
                snapFile.write(data.data, data.raw_size);
                snapFile.close();
            }
            return true;
        }
    }
}