// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_EXTERNAL_REGISTRY_H__
#define __V8_EXTERNAL_REGISTRY_H__

#include <vector>

#include "CppBridge/V8FunctionTemplate.h"
#include "Utils/CallbackWrapper.h"

namespace v8App
{
    namespace JSRuntime
    {
        using GlobalTemplateRegisterFunction = void (*)(V8Isolate *inIsolate, v8::Local<v8::ObjectTemplate> &inGlobal);

        class V8ExternalRegistry
        {
        public:
            static const std::vector<intptr_t> &GetReferences();
            template <typename Signature>
            static void Register(Utils::CallbackWrapper<Signature> inCallback)
            {
                V8ExternalRegistry* instance = GetInstance();
                // if the register has the nullptr at the end then pop it off so we can add to it
                if (instance->m_Registry.size() != 0 && instance->m_Registry.back() == 0)
                {
                    instance->m_Registry.pop_back();
                }
                intptr_t funcAddress = reinterpret_cast<intptr_t>(&CppBridge::CallbackDispatcher<Signature>::V8CallbackForFunction);
                // if the address isn't already aded then add it
                if (std::find(instance->m_Registry.begin(), instance->m_Registry.end(), funcAddress) == instance->m_Registry.end())
                {
                    instance->m_Registry.push_back(funcAddress);
                }
            }

            static void RegisterGlobalRegisterer(GlobalTemplateRegisterFunction inRegister);
            static void RunGlobalRegisterFunctions(V8Isolate *inIsolate, v8::Local<v8::ObjectTemplate> &inGlobal);

        protected:
            static V8ExternalRegistry* GetInstance();
            static std::unique_ptr<V8ExternalRegistry> s_Instance;

            std::vector<intptr_t> m_Registry;
            std::vector<GlobalTemplateRegisterFunction> m_RegisterFunctions;
        };
    }
}

#define CONCAT_REG(a, b) a##b
#define REGISTER_FUNCS(Name)                               \
    namespace Registration                                 \
    {                                                      \
        static void CONCAT_REG(register_funcs_, Name)();   \
        namespace                                          \
        {                                                  \
            struct CONCAT_REG(register_struct_, Name)      \
            {                                              \
                CONCAT_REG(register_struct_, Name)         \
                ()                                         \
                {                                          \
                    CONCAT_REG(register_funcs_, Name)      \
                    ();                                    \
                }                                          \
            } CONCAT_REG(register_struct_instance_, Name); \
        }                                                  \
    }                                                      \
    void Registration::CONCAT_REG(register_funcs_, Name)()

#endif
