// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <algorithm>

#include "V8ExternalRegistry.h"

namespace v8App
{
    namespace JSRuntime
    {
        void V8ExternalRegistry::Register(void *inAddress)
        {
            intptr_t intAddress = reinterpret_cast<intptr_t>(inAddress);
            if (std::find(m_Registry.begin(), m_Registry.end(), intAddress) == m_Registry.end())
            {
                //registry always has to end in nullptr so replace it and push it back on the end
                int index = m_Registry.size() == 0 ? 0 : m_Registry.size() - 1;
                if (index == 0)
                {
                    m_Registry.push_back(intAddress);
                }
                else
                {
                    m_Registry[index] = intAddress;
                }
                m_Registry.push_back(reinterpret_cast<intptr_t>(nullptr));
            }
        }

        const std::vector<intptr_t> &V8ExternalRegistry::GetReferences()
        {
            return m_Registry;
        }
    } // namespace JSRuntime

}