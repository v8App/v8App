// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef _JSAPP_CREATOR_REGISTRY_H_
#define _JSAPP_CREATOR_REGISTRY_H_

#include <string>
#include <map>

#include "V8Types.h"

namespace v8App
{
    namespace JSRuntime
    {
        /**
         * Class to register creators to create app subclasses from snapshots.
         * App Subclass has to be registered to be looaded
         */
        class JSAppCreatorRegistry
        {
        public:
            using JSAppCreatorFunction = JSAppSharedPtr (*)();
            static void RegisterCreator(std::string inAppType, JSAppCreatorFunction inCreator);

            static JSAppSharedPtr CreateApp(std::string inAppType);

        protected:
            static JSAppCreatorRegistry* GetInstance();
            inline static std::shared_ptr<JSAppCreatorRegistry> s_Instance{nullptr};

            std::map<std::string, JSAppCreatorFunction> m_Creators;
        };
    }
}

/**
 * Macro to register the jsApp type with the JSApp creator class below
 */
#define CONCAT_REG(a, b) a##b
#define REGISTER_JSAPP_CREATOR(Name, creator)                                                                   \
    namespace Registration                                                                                      \
    {                                                                                                           \
        namespace                                                                                               \
        {                                                                                                       \
            struct CONCAT_REG(register_app_struct_, Name)                                                       \
            {                                                                                                   \
                CONCAT_REG(register_app_struct_, Name)()                                                        \
                {                                                                                               \
                    Name app;                                                                                  \
                    v8App::JSRuntime::JSAppCreatorRegistry::RegisterCreator(app.GetClassType(), creator);      \
                }                                                                                               \
            } CONCAT_REG(egister_app_struct_instance_, Name);                                                   \
        }                                                                                                       \
    }

#endif //_JSAPP_CREATOR_REGISTRY_H_