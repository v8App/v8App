// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>

#include "Utils/Format.h"

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8FunctionTemplate.h"
#include "CppBridge/V8CppObjInfo.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            std::unique_ptr<CallbackRegistry> CallbackRegistry::s_Instance;

            CallbackRegistry *CallbackRegistry::GetInstance()
            {
                if (s_Instance == nullptr)
                {
                    s_Instance = std::make_unique<CallbackRegistry>();
                }
                return s_Instance.get();
            }

            const std::vector<intptr_t> &CallbackRegistry::GetReferences()
            {
                CallbackRegistry *instance = GetInstance();
                if (instance->m_Registry.size() == 0 || instance->m_Registry.back() != 0)
                {
                    instance->m_Registry.push_back((intptr_t) nullptr);
                }
                return instance->m_Registry;
            }

            void CallbackRegistry::AddNamespaceSetupFunction(GlobalTemplateRegisterFunction inRegister, std::vector<std::string> inNamespaces)
            {
                if (inNamespaces.size() == 0)
                {
                    inNamespaces.push_back(CallbackRegistry::GlobalNamespace);
                }

                CallbackRegistry *instance = GetInstance();
                for (auto name : inNamespaces)
                {
                    instance->m_RegisterFunctions[name].push_back(inRegister);
                }
            }

            void CallbackRegistry::RunNamespaceSetupFunctions(JSContextSharedPtr inContext, V8LObject &inGlobal, std::string inNamespace)
            {
                CallbackRegistry *instance = GetInstance();
                std::vector<std::string> namespaces{CallbackRegistry::GlobalNamespace};
                if(inNamespace != "")
                {
                    namespaces.push_back(inNamespace);
                }
                for (auto &name : namespaces)
                {
                    if (instance->m_RegisterFunctions.count(name) == 0)
                    {
                        continue;
                    }
                    for (auto func : instance->m_RegisterFunctions[name])
                    {
                        func(inContext, inGlobal);
                    }
                }
            }

            bool CallbackRegistry::DoesNamespaceExistInRegistry(std::string inNamespace)
            {
                CallbackRegistry *instance = GetInstance();
                return instance->m_RegisterFunctions.count(inNamespace) > 0;
            }

            void CallbackRegistry::RegisterObjectInfo(V8CppObjInfo *inInfo)
            {
                CallbackRegistry *instance = GetInstance();
                for (auto it : instance->m_ObjectInfos)
                {
                    if (it.second == inInfo)
                    {
                        Log::LogMessage msg = {
                            {Log::MsgKey::Msg, Utils::format("ObjectInfo with typename {} already registered", inInfo->m_TypeName)}};
                        LOG_WARN(msg);
                        return;
                    }
                }
                if (instance->m_ObjectInfos.contains(inInfo->m_TypeName) && instance->m_ObjectInfos[inInfo->m_TypeName] != inInfo)
                {
                    Log::LogMessage msg = {
                        {Log::MsgKey::Msg, Utils::format("ObjectInfo with typename {} already registered but with different info struct", inInfo->m_TypeName)}};
                    LOG_FATAL(msg);
                    return;
                }
                instance->m_ObjectInfos[inInfo->m_TypeName] = inInfo;
            }

            V8CppObjInfo *CallbackRegistry::GetNativeObjectInfoFromTypeName(std::string inTypeName)
            {
                CallbackRegistry *instance = GetInstance();
                if (instance->m_ObjectInfos.contains(inTypeName))
                {
                    return instance->m_ObjectInfos[inTypeName];
                }
                return nullptr;
            }
        }
    }
} // namespace JSRuntime
