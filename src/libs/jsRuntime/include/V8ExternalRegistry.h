// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __V8_EXTERNAL_REGISTRY_H__
#define __V8_EXTERNAL_REGISTRY_H__

#include <vector>

namespace v8App
{
    namespace JSRuntime
    {
        using AllowedFunctionCallbackOneByteString = 
        class V8ExternalRegistry
        {
        public:
            V8ExternalRegistry() = default;
            ~V8ExternalRegistry() = default;


            const std::vector<intptr_t> &GetReferences();

        protected:
            template<typename T>
            void Register(T* inAdress)
            {
                m_Registry.push_back(reinterpret_cast<intptr_t>(inAddress));
            }
            std::vector<intptr_t> m_Registry;
        };
    }
}
#endif
