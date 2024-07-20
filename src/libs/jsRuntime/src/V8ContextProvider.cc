// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "uuid/uuid.h"

#include "CppBridge/CallbackRegistry.h"
#include "V8ContextProvider.h"
#include "JSRuntime.h"
#include "JSContext.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContextSharedPtr V8ContextProvider::CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
                                                            std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot,
                                                            SnapshotMethod inSnapMethod, size_t inContextIndex)
        {
            // if the namespace is empty, ie default v8 context, and it doesn't exist then error
            // TODO: need to move the bridge out of the ccontext
            if (inNamespace != "" && CppBridge::CallbackRegistry::DoesNamespaceExistInRegistry(inNamespace) == false)
            {
                // TODO: log message
                return nullptr;
            }

            JSContextSharedPtr context = std::make_shared<JSContext>(inRuntime, inName, inNamespace, inEntryPoint, inContextIndex,
                                                                     inSnapEntryPoint, inSupportsSnapshot, inSnapMethod);
            if (context->CreateContext() == false)
            {
                context->DisposeContext();
                return nullptr;
            }

            V8Isolate * isolate = inRuntime->GetIsolate();

            V8IsolateScope iScope(isolate);
            V8HandleScope hScope(isolate);
            V8LContext v8Context = context->GetLocalContext();
            V8ContextScope cScope(v8Context);

            V8LObject globalObj = v8Context->Global();
            CppBridge::CallbackRegistry::RunNamespaceSetupFunctions(context, globalObj, inNamespace);

            // if the runtime this is created for is a snapshotter and the method is namespace only
            // then return
            if (inRuntime->IsSnapshotRuntime() && inSnapMethod == SnapshotMethod::kNamespaceOnly)
            {
                return context;
            }

            //if the context index is not 0 then no need to run the entry point
            if(context->GetSnapshotIndex() != 0)
            {
                return context;
            }

            if (context->RunEntryPoint(false) == false)
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