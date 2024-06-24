// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "uuid/uuid.h"


#include "CppBridge/CallbackRegistry.h"
#include "JSContextCreator.h"
#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        JSContextSharedPtr JSContextCreator::CreateContext(JSRuntimeSharedPtr inRuntime, std::string inName, std::string inNamespace,
            std::filesystem::path inEntryPoint, std::filesystem::path inSnapEntryPoint, bool inSupportsSnapshot, 
                      SnapshotMethod inSnapMethod)
        {
            size_t index = GetSnapIndexForNamespace(inNamespace);
            if (index == JSRuntime::kMaxContextNamespaces)
            {
                // if the index is max then set it to 0 since the snapshot doesn't have it and
                // we'll create the global object for it then
                index = 0;
            }

            // if the namespace is empty, ie default v8 context, and it doesn't exist then error
            // TODO: need to move the bridge out of the ccontext
            if (inNamespace != "" && CppBridge::CallbackRegistry::DoesNamespaceExistInRegistry(inNamespace) == false)
            {
                return false;
            }

            JSContextSharedPtr context = std::make_shared<JSContext>(inRuntime, inName, inNamespace, inEntryPoint, inSnapEntryPoint, inSupportsSnapshot, inSnapMethod);
            if (context->CreateContext(index) == false)
            {
                context->DisposeContext();
                return nullptr;
            }
            if (m_Namespace != "")
            {
                v8::Isolate::Scope(inRuntome->GetIsolate();
                v8::HandleScope hScope(inRuntime->GetIsolate());
                V8LocalContext v8Context = context->GetLocalContext();
                v8::Context::Scope(v8Context);
                CppBridge::CallbackRegistry::RunNamespaceSetupFunctions(context, v8COntext->Global(), m_Namespace);
            }
            return context;
        }

        void JSContextCreator::DisposeContext(JSContextSharedPtr inContext)
        {
            inContext->DisposeContext();
        }

        void JSContextCreator::RegisterSnapshotCloser(JSContextSharedPtr inContext)
        {
            inContext->RegisterSnapshotCloser();
        }

        void JSContextCreator::UnregisterSnapshotCloser(JSContextSharedPtr inContext)
        {
            inContext->UnregisterSnapshotCloser();
        }
    }
}