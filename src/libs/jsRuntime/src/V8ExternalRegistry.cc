// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>

#include "V8ExternalRegistry.h"
#include "CppBridge/V8FunctionTemplate.h"

namespace v8App
{
    namespace JSRuntime
    {

        std::unique_ptr<V8ExternalRegistry> V8ExternalRegistry::s_Instance;

        V8ExternalRegistry *V8ExternalRegistry::GetInstance()
        {
            if (s_Instance == nullptr)
            {
                s_Instance = std::make_unique<V8ExternalRegistry>();
            }
            return s_Instance.get();
        }

        const std::vector<intptr_t> &V8ExternalRegistry::GetReferences()
        {
            V8ExternalRegistry *instance = GetInstance();
            if (instance->m_Registry.size() == 0 || instance->m_Registry.back() != 0)
            {
                instance->m_Registry.push_back((intptr_t) nullptr);
            }
            return instance->m_Registry;
        }

        void V8ExternalRegistry::RegisterGlobalRegisterer(GlobalTemplateRegisterFunction inRegister)
        {
            V8ExternalRegistry *instance = GetInstance();
            if (std::find(instance->m_RegisterFunctions.begin(), instance->m_RegisterFunctions.end(), inRegister) == instance->m_RegisterFunctions.end())
            {
                instance->m_RegisterFunctions.push_back(inRegister);
            }
        }

        void V8ExternalRegistry::RunGlobalRegisterFunctions(V8Isolate *inIsolate, v8::Local<v8::ObjectTemplate> &inGlobal)
        {
            V8ExternalRegistry *instance = GetInstance();
            for (auto func : instance->m_RegisterFunctions)
            {
                func(inIsolate, inGlobal);
            }
        }

    } // namespace JSRuntime

}