// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __CALLBACK_REGISTRY_H__
#define __CALLBACK_REGISTRY_H__

#include <vector>

#include "V8Types.h"
#include "Utils/CallbackWrapper.h"
#include "CppBridge/CallbackHolderBase.h"

namespace v8App
{
    namespace JSRuntime
    {
        namespace CppBridge
        {
            /**
             * Forward declare
             */
            template <typename Signature>
            class CallbackHolder;
            template <typename Signature>
            struct CallbackDispatcher;
            struct V8NativeObjectInfo;

            /**
             * The sginature of the function to that can be regiseterd so functions and object templates can be
             * registered on the isolate
             */
            using GlobalTemplateRegisterFunction = void (*)(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal);

            /**
             * Singleton for handling all of the function registration used for looking up real function calls and passing the
             * calls registerd with V8. Has an internal instance of the registry because of SIOF(Static Initialization Order Fiasco)
             *
             * How it works
             * You register a function or member function with the Register function.
             * It Store it callback holder object with the passed function's address.
             *
             * The Callback dispatcher's static fucntion is set as the V8 callback and the real functions
             * address is stored as a BigInt in the data of the function template.
             * When V8 calls the function it looks up this holder, unpacks the arguments and invokes the callback with them
             *
             * ExternalReferences
             * For snapshotting the external references are really the templated class for the function signature so
             * any function with the same signature will actually share the same callback so there will be far less
             * of them than in the callback holders map.
             */
            class CallbackRegistry
            {
            public:
                /**
                 * Gets the4 ensteral references that V8 will need for snapshotting
                 */
                static const std::vector<intptr_t> &GetReferences();

                /**
                 * Registers the function and it's callback holder for lookup later when V8 dispatches to it
                 */
                template <typename Signature>
                static void Register(Utils::CallbackWrapper<Signature> inCallback, const char *TypeName = "")
                {
                    using HolderInstType = CallbackHolder<Signature>;
                    size_t realFuncAddress = inCallback.GetFuncAddress();
                    intptr_t v8FuncAddress = reinterpret_cast<intptr_t>(&CallbackDispatcher<Signature>::V8CallbackForFunction);

                    CallbackRegistry *instance = GetInstance();
                    // if the register has the nullptr at the end then pop it off so we can add to it
                    if (instance->m_Registry.size() != 0 && instance->m_Registry.back() == 0)
                    {
                        instance->m_Registry.pop_back();
                    }
                    // if the address isn't already aded then add it
                    if (std::find(instance->m_Registry.begin(), instance->m_Registry.end(), v8FuncAddress) == instance->m_Registry.end())
                    {
                        instance->m_Registry.push_back(v8FuncAddress);
                    }

                    if (instance->m_CallbackHolders.find(realFuncAddress) == instance->m_CallbackHolders.end())
                    {
                        HolderInstType *holder = new HolderInstType(std::move(inCallback), TypeName);
                        instance->m_CallbackHolders.emplace(realFuncAddress, holder);
                    }
                }

                /**
                 * Looks up the specified callback holder so that V8 Callback dispatcher can call the real function
                 */
                static CallbackHolderBase *GetCallbackHolder(size_t inFuncAddress)
                {
                    CallbackRegistry *instance = GetInstance();
                    auto it = instance->m_CallbackHolders.find(inFuncAddress);
                    if (it == instance->m_CallbackHolders.end())
                    {
                        return nullptr;
                    }
                    return it->second;
                }

                /**
                 * Adds a global template registration function that register functions/objects templates
                 * These get called by JSRuntime when setting up the isolates global template
                 */
                static void RegisterGlobalRegisterer(GlobalTemplateRegisterFunction inRegister);
                /**
                 * Runs the registered callbacks on the provided isolate
                 */
                static void RunGlobalRegisterFunctions(JSRuntimeSharedPtr inRuntime, v8::Local<v8::ObjectTemplate> &inGlobal);

                /**
                 * Registers a cpp class ObjectInfo. This is used with snapshot restoration to lookup the
                 * deserializer for the class data
                 */
                static void RegisterObjectInfo(V8NativeObjectInfo *inInfo);
                /**
                 * Finds the specified class's object info
                 */
                static V8NativeObjectInfo *GetNativeObjectInfoFromTypeName(std::string inTypeName);

            protected:
                static CallbackRegistry *GetInstance();
                static std::unique_ptr<CallbackRegistry> s_Instance;

                std::vector<intptr_t> m_Registry;
                std::map<size_t, CallbackHolderBase *> m_CallbackHolders;
                std::vector<GlobalTemplateRegisterFunction> m_RegisterFunctions;
                std::map<std::string, V8NativeObjectInfo *> m_ObjectInfos;
            };
        }
    }
}

/**
 * Macro to static register fucntions on the callback registry.
 * Youe just need to useit then put the code to run.
 * REGISTER_FUNCS(Test)
 * {
 *  **  run registeration code here
 * }}
 */
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

/**
 * Class registration version of the above but call to a static
 * functi0on in the class to do the registration
 */
#define REGISTER_CLASS_FUNCS(ClassName)                                                 \
    namespace Registration                                                              \
    {                                                                                   \
        static void CONCAT_REG(register_funcs_, ClassName)();                           \
        namespace                                                                       \
        {                                                                               \
            struct CONCAT_REG(register_struct_, ClassName)                              \
            {                                                                           \
                CONCAT_REG(register_struct_, ClassName)                                 \
                ()                                                                      \
                {                                                                       \
                    CONCAT_REG(register_funcs_, ClassName)                              \
                    ();                                                                 \
                }                                                                       \
            } CONCAT_REG(register_struct_instance_, ClassName);                         \
        }                                                                               \
    }                                                                                   \
    void Registration::CONCAT_REG(register_funcs_, ClassName)()                         \
    {                                                                                   \
        ClassName::RegisterClassFunctions();                                            \
        CppBridge::CallbackRegistry::RegisterGlobalRegisterer(&ClassName::RegisterGlobalTemplate); \
    }

#endif //__CALLBACK_REGISTRY_H__
