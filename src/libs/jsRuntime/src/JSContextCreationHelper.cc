// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Serialization/TypeSerializer.h"

#include "JSContextCreationHelper.h"
#include "JSRuntime.h"

namespace v8App
{
    namespace JSRuntime
    {
        bool JSContextCreationHelper::SetContextNamespaces(V8StartupData *inData)
        {
            Serialization::ReadBuffer rBuffer(inData->data, inData->raw_size);

            if (rBuffer.AtEnd())
            {
                return false;
            }

            size_t numNamespaces;
            rBuffer >> numNamespaces;
            for (size_t i = 0; i < numNamespaces; i++)
            {
                size_t index;
                std::string name;
                rBuffer >> index;
                rBuffer >> name;
                if (rBuffer.HasErrored())
                {
                    return false;
                }
                auto [it, success] = m_NamespacesSnapIndexes.insert(std::make_pair((int)index, name));
                if (success == false)
                {
                    return false;
                }
            }
            return true;
        }

        V8StartupData JSContextCreationHelper::SerializeContextNamespaces()
        {
            V8StartupData v8Data;
            Serialization::WriteBuffer wBuffer;

            wBuffer << m_NamespacesSnapIndexes.size();
            for (auto it : m_NamespacesSnapIndexes)
            {
                size_t index = it.first;
                std::string name = it.second;
                wBuffer << index;
                wBuffer << name;
            }
            v8Data.data = wBuffer.GetDataNew();
            v8Data.raw_size = wBuffer.BufferSize();
            return v8Data;
        }

        bool JSContextCreationHelper::AddSnapIndexNamespace(size_t inSnapIndex, std::string inNamespace)
        {
            /**
             * No namespace defaults to the default v8 context
             */
            if (inNamespace == "")
            {
                return true;
            }
            for (auto [index, name] : m_NamespacesSnapIndexes)
            {
                if (index == inSnapIndex || name == inNamespace)
                {
                    return false;
                }
            }
            auto [it, success] = m_NamespacesSnapIndexes.insert(std::make_pair((int)inSnapIndex, inNamespace));
            return success;
        }

        std::string JSContextCreationHelper::GetNamespaceForSnapIndex(size_t inSnapIndex)
        {
            if (m_NamespacesSnapIndexes.count(inSnapIndex) > 0)
            {
                return m_NamespacesSnapIndexes[inSnapIndex];
            }
            return std::string();
        }

        size_t JSContextCreationHelper::GetSnapIndexForNamespace(std::string inNamespace)
        {
            for (auto [index, name] : m_NamespacesSnapIndexes)
            {
                if (name == inNamespace)
                {
                    return index;
                }
            }
            return JSRuntime::kMaxContextNamespaces;
        }
    }
}