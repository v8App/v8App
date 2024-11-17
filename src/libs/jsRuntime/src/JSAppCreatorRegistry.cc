// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Utils/Format.h"
#include "Logging/LogMacros.h"

#include "JSAppCreatorRegistry.h"

namespace v8App
{
    namespace JSRuntime
    {
        void JSAppCreatorRegistry::RegisterCreator(std::string inAppType, JSAppCreatorRegistry::JSAppCreatorFunction inCreator)
        {
            JSAppCreatorRegistry *instance = GetInstance();

            // we onlt check that the type name is not registered and
            // allow the same create to be resitered with a different type
            auto it = instance->m_Creators.find(inAppType);
            if (it != instance->m_Creators.end())
            {
                // if it's the same creator then it's fine
                if (it->second == inCreator)
                {
                    return;
                }
                LOG_WARN(Utils::format("Found an app already registered with class Type '{}'", inAppType));
                return;
            }
            auto [dummy, success] = instance->m_Creators.insert(std::make_pair(inAppType, inCreator));
            if (success == false)
            {
                LOG_WARN(Utils::format("Failed to add app of class type  '{}' to the registry", inAppType));
            }
            return;
        }

        JSAppSharedPtr JSAppCreatorRegistry::CreateApp(std::string inAppType)
        {
            JSAppCreatorRegistry *instance = GetInstance();

            auto it = instance->m_Creators.find(inAppType);
            if (it == instance->m_Creators.end())
            {
                return JSAppSharedPtr();
            }
            return it->second();
        }

        JSAppCreatorRegistry *JSAppCreatorRegistry::GetInstance()
        {
            if (s_Instance == nullptr)
            {
                s_Instance = std::make_shared<JSAppCreatorRegistry>();
            }
            return s_Instance.get();
        }
    }
}