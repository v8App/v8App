// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "uuid/uuid.h"
#include "Utils/Format.h"

#include "CppBridge/CallbackRegistry.h"
#include "V8ContextProvider.h"
#include "JSRuntime.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContextSharedPtr V8ContextProvider::CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
                                                            std::filesystem::path inEntryPoint, bool inSupportsSnapshot,
                                                            SnapshotMethod inSnapMethod, size_t inContextIndex)
        {
            V8Isolate::Scope iScope(inRuntime->GetIsolate());
            V8HandleScope hScope(inRuntime->GetIsolate());
            V8TryCatch tryCatch(inRuntime->GetIsolate());

            // if the namespace is empty, ie default v8 context, and it doesn't exist then error
            // TODO: need to move the bridge out of the ccontext
            if (inNamespace != "" && CppBridge::CallbackRegistry::DoesNamespaceExistInRegistry(inNamespace) == false)
            {
                LOG_ERROR(Utils::format("Context Namespace doesn't exust, Namespace:{}", inNamespace));
                return nullptr;
            }
            // register all the namespace's function templates
            CppBridge::CallbackRegistry::RunNamespaceSetupFunctions(inRuntime, inNamespace);

            JSContextSharedPtr context = std::make_shared<JSContext>(inRuntime, inName, inNamespace, inEntryPoint, inContextIndex,
                                                                     inSupportsSnapshot, inSnapMethod);
            if (context->CreateContext() == false)
            {
                context->DisposeContext();
                return nullptr;
            }

            // if the runtime this is created for is a snapshotter and the method is namespace only
            // then return
            if (inRuntime->IsSnapshotRuntime() && inSnapMethod == SnapshotMethod::kNamespaceOnly)
            {
                return context;
            }

            // if the context index is not 0 then no need to run the entry point
            if (context->GetSnapshotIndex() > 0)
            {
                return context;
            }
            context->RunEntryPoint(false).IsEmpty();
            if (tryCatch.HasCaught())
            {
                context->DisposeContext();
                return nullptr;
            }

            return context;
        }

        void V8ContextProvider::DisposeContext(JSContextSharedPtr inContext)
        {
            inContext->DisposeContext();
        }
    }
}