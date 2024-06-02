// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>

#include "CppBridge/CallbackRegistry.h"
#include "CppBridge/V8FunctionTemplate.h"
#include "CppBridge/V8NativeObject.h"

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

            void CallbackRegistry::RegisterGlobalRegisterer(GlobalTemplateRegisterFunction inRegister)
            {
                CallbackRegistry *instance = GetInstance();
                if (std::find(instance->m_RegisterFunctions.begin(), instance->m_RegisterFunctions.end(), inRegister) == instance->m_RegisterFunctions.end())
                {
                    instance->m_RegisterFunctions.push_back(inRegister);
                }
            }

            void CallbackRegistry::RunGlobalRegisterFunctions(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal)
            {
                CallbackRegistry *instance = GetInstance();
                for (auto func : instance->m_RegisterFunctions)
                {
                    func(inRuntime, inGlobal);
                }
            }

            void CallbackRegistry::RegisterObjectInfo(V8NativeObjectInfo *inInfo)
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

            V8NativeObjectInfo *CallbackRegistry::GetNativeObjectInfoFromTypeName(std::string inTypeName)
            {
                CallbackRegistry *instance = GetInstance();
                if (instance->m_ObjectInfos.contains(inTypeName))
                {
                    return instance->m_ObjectInfos[inTypeName];
                }
                return nullptr;
            }

        }
    } // namespace JSRuntime
}