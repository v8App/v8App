// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <string>

namespace v8App
{
    namespace Utils
    {
        /**
         * Gets an environment variable for the OS
        */
        inline std::string GetEnvironmentVar(std::string EnvVar)
        {
            const char* var = std::getenv(EnvVar.c_str());
            if(var == nullptr) {
                return std::string();
            }
            return std::string(var);
        }
    } // Utils
} // v8App

#endif //__ENVIRONMENT_H__