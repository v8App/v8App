// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Utils/Format.h"

#include "JSSnapshotCreator.h"
#include "JSContext.h"

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
            v8::SnapshotCreator *creator = m_App->GetSnapshotCreator();
            if (creator == nullptr)
            {
                return false;
            }

            {
                v8::HandleScope hScope(m_App->GetJSRuntime()->GetIsolate());

                //JSContextSharedPtr defaultContext = m_App->CreateJSContext("default");
                //V8LocalContext lContext = defaultContext->GetLocalContext();
                V8LocalContext lContext = v8::Context::New(m_App->GetJSRuntime()->GetIsolate());
                bool empty = lContext.IsEmpty();
                creator->SetDefaultContext(lContext, v8::SerializeEmbedderFieldsCallback(JSSnapshotCreator::SerializeInternalField));
            }

            V8Platform::Get()->SetWorkersPaused(true);
            v8::StartupData data = creator->CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kClear);
            if (data.raw_size == 0)
            {
                V8Platform::Get()->SetWorkersPaused(false);
                return false;
            }
            V8Platform::Get()->SetWorkersPaused(false);

            std::ofstream snapFile(inSnapshotFile, std::ios::out | std::ios::binary);
            if (snapFile.is_open() == false || snapFile.fail())
            {
                return false;
            }
            snapFile.write(data.data, data.raw_size);
            snapFile.close();
            return true;

            /**
                    JSRuntimeSharedPtr runtime = m_App->CreateJSRuntime();
                    v8::SnapshotCreator creator(runtime->GetIsolate(), nullptr, nullptr, true);

                    // the default context with just the v8 builtins
                    JSContextSharedPtr defaultContext = runtime->CreateContext("SnapshotDefault");
                    creator.SetDefaultContext(defaultContext->GetLocalContext());

                    // the app context with all the v8App setup
                    JSContextSharedPtr appContext = runtime->CreateContext("SnapshotV8App");
                    {
                        V8LocalContext context = appContext->GetLocalContext();
                        v8::Context::Scope cScope(context);
                        // do what ever setup we need to do before the entry point is loaded

                        JSContextModulesSharedPtr jsModules = appContext->GetJSModules();
                        JSModuleInfoSharedPtr module = jsModules->LoadModule(m_App->GetEntryPointScript());
                        if (module == nullptr)
                        {
                            Log::LogMessage msg = {
                                {Log::MsgKey::Msg, Utils::format("Failed to load the entry point script. Script:{}", m_App->GetEntryPointScript())}};
                            LOG_ERROR(msg);
                            return false;
                        }
                        if (jsModules->InstantiateModule(module) == false)
                        {
                            Log::LogMessage msg = {
                                {Log::MsgKey::Msg, Utils::format("Failed to intantiate the entry point script. Script:{}", m_App->GetEntryPointScript())}};
                            LOG_ERROR(msg);
                            return false;
                        }
                        if (jsModules->RunModule(module) == false)
                        {
                            Log::LogMessage msg = {
                                {Log::MsgKey::Msg, Utils::format("Failed to run the entry point script. Script:{}", m_App->GetEntryPointScript())}};
                            LOG_ERROR(msg);
                            return false;
                        }
                        creator.AddContext(context, v8::SerializeInternalFieldsCallback(JSSnapshotCreator::SerializeInternalField));
                    }

                    // todo figure out what mode or cercumstances to use each
                    v8::StartupData blob = creator.CreateBlob(v8::SnapshotCreator::FunctionCodeHandling::kKeep);

                    if (blob.data == nullptr)
                    {
                        Log::LogMessage msg = {
                            {Log::MsgKey::Msg, "Failed to serialize the runtime."}};
                        LOG_ERROR(msg);
                        return false;
                    }

                    if (blob.CanBeRehashed() == false)
                    {
                        Log::LogMessage msg = {
                            {Log::MsgKey::Msg, "Failed to serialize the runtime, blob can't be rehashed."}};
                        LOG_ERROR(msg);
                        return false;
                    }

                    std::ofstream blobFile(inSnapshotFile, std::ios::out | std::ios::binary);
                    if (blobFile.is_open() == false || blobFile.fail())
                    {
                        Log::LogMessage msg = {
                            {Log::MsgKey::Msg, Utils::format("Failed to serialize the runtime to {}", inSnapshotFile)}};
                        LOG_ERROR(msg);
                        return false;
                    }

                    blobFile.write(blob.data, blob.raw_size);
                    if (blobFile.fail())
                    {
                        Log::LogMessage msg = {
                            {Log::MsgKey::Msg, Utils::format("Failed to write the snapshot data to {}", inSnapshotFile)}};
                        LOG_ERROR(msg);
                        return false;
                    }
                    return true;
                    */
        }

        v8::StartupData JSSnapshotCreator::SerializeInternalField(V8LocalObject inHolder, int inIndex, void *inData)
        {
            JSRuntimeWeakPtr *ptr = static_cast<JSRuntimeWeakPtr *>(inHolder->GetAlignedPointerFromInternalField(inIndex));
            return {nullptr, 0};
        }

        void JSSnapshotCreator::DeserializeInternalField(V8LocalObject inHolder, int inINdex, v8::StartupData inPayload, void *inData)
        {
        }
    }
}