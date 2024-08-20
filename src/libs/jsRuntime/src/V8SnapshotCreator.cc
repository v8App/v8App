// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "JSApp.h"
#include "JSContext.h"
#include "JSRuntime.h"
#include "V8SnapshotCreator.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool V8SnapshotCreator::CreateSnapshot(ISnapshotObject &inObject, std::filesystem::path inSnapshotFile)
        {
            if (inSnapshotFile.empty())
            {
                LOG_ERROR("CreateSnapshot passed an empty file path");
                return false;
            }

            JSRuntime *runtime = dynamic_cast<JSRuntime *>(&inObject);
            JSApp *app = nullptr;
            if (runtime == nullptr)
            {
                app = dynamic_cast<JSApp *>(&inObject);
                if (app == nullptr)
                {
                    LOG_ERROR("CreateSnapshot epxects to be passed a JSRuntime or JSApp object");
                    return false;
                }
                runtime = app->GetMainRuntime().get();
                if (runtime == nullptr)
                {
                    LOG_ERROR("CreateSNaphot failed to get the main context from the app");
                    return false;
                }
            }
            if (runtime->IsInitialzed() || runtime->IsSnapshotRuntime() == false)
            {
                LOG_ERROR(Utils::format("The runtime {} is not snapshottable", runtime->GetName()));
                return false;
            }

            V8SnapshotCreatorSharedPtr creater = runtime->GetSnapshotCreator();

            V8Isolate *isolate = runtime->GetIsolate();
            {
                V8HandleScope hScope(isolate);
                V8LContext context = V8Context::New(isolate);
                creater->SetDefaultContext(context);
            }

            runtime->CloseOpenHandlesForSnapshot();
            V8StartupData data = creater->CreateBlob(V8SnapCreator::FunctionCodeHandling::kClear);
            if (data.raw_size == 0)
            {
                LOG_ERROR(Utils::format("Failed to create the snapshot for runtime {}", runtime->GetName()));
                return false;
            }

            std::ofstream snapFile(inSnapshotFile, std::ios::out | std::ios::binary);
            if (snapFile.is_open() == false || snapFile.fail())
            {
                return false;
            }
            snapFile.write(data.data, data.raw_size);
            snapFile.close();

            return true;
        }
    }
}