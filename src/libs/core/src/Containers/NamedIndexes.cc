// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Serialization/ReadBuffer.h"
#include "Serialization/WriteBuffer.h"
#include "Serialization/TypeSerializer.h"

#include "Containers/NamedIndexes.h"

namespace v8App
{
    namespace Containers
    {
        std::string NamedIndexes::GetNameFromIndex(size_t inIndex)
        {
            if (m_NamedIndexes.count(inIndex) > 0)
            {
                return m_NamedIndexes[inIndex];
            }
            return std::string();
        }

        size_t NamedIndexes::GetIndexForName(std::string inName)
        {
            for (auto [index, name] : m_NamedIndexes)
            {
                if (name == inName)
                {
                    return index;
                }
            }
            return NamedIndexes::m_MaxNmaedIndexes;
        }

        bool NamedIndexes::DeserializeNameIndexes(Serialization::ReadBuffer &inBuffer)
        {
            if (inBuffer.AtEnd())
            {
                return false;
            }

            size_t numNamespaces;
            inBuffer >> m_MaxNmaedIndexes;
            inBuffer >> numNamespaces;
            for (size_t i = 0; i < numNamespaces; i++)
            {
                size_t index;
                std::string name;
                inBuffer >> index;
                inBuffer >> name;
                if (inBuffer.HasErrored())
                {
                    return false;
                }
                auto [it, success] = m_NamedIndexes.insert(std::make_pair((int)index, name));
                if (success == false)
                {
                    return false;
                }
            }
            return true;
        }

        bool NamedIndexes::SerializeNameIndexes(Serialization::WriteBuffer &inBuffer)
        {
            inBuffer << m_MaxNmaedIndexes;
            inBuffer << m_NamedIndexes.size();
            for (auto it : m_NamedIndexes)
            {
                size_t index = it.first;
                std::string name = it.second;
                inBuffer << index;
                inBuffer << name;
            }
            return true;
        }

        bool NamedIndexes::AddNamedIndex(size_t inIndex, std::string inName)
        {
            /**
             * No namespace defaults to the default v8 context
             */
            if (inName == "")
            {
                // TODO: log message
                return false;
            }

            for (auto [index, name] : m_NamedIndexes)
            {
                if (index == inIndex || name == inName)
                {
                    // TODO: log message
                    return false;
                }
            }
            auto [it, success] = m_NamedIndexes.insert(std::make_pair(inIndex, inName));
            return success;
        }
    }
}
