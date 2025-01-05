// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __NAMED_INDEXES_H__
#define __NAMED_INDEXES_H__

#include <map>

#include "Serialization/TypeSerializer.h"

namespace v8App
{
    namespace Serialization
    {
        class ReadBuffer;
        class WriteBuffer;
    }

    namespace Containers
    {
        /**
         * This maps a int to a string for use where we store/retrieve that info from a buffer
         * Note: The indexes don't have to be contiguous
         */
        class NamedIndexes
        {
        public:
            NamedIndexes(size_t inMaxIndexesAllowed = 1024) : m_MaxNmaedIndexes(inMaxIndexesAllowed) {}
            ~NamedIndexes() = default;

            /**
             * Gets the name for the associated index.
             * If the index doesn't exists then returns empty string
             */
            std::string GetNameFromIndex(size_t inINdex);
            /**
             * Gets the index for the given name
             * If the name doesn't exists then returns the max supported named indexes
             */
            size_t GetIndexForName(std::string inName);

            /**
             * Serializes the name index info into the specified buffer
             */
            bool DeserializeNameIndexes(Serialization::ReadBuffer &inBuffer);
            /**
             * Desrializes the name index info from the specified buffer
             */
            bool SerializeNameIndexes(Serialization::WriteBuffer &inBuffer) const;
            /**
             * Addes an index for the namae.
             * If the name or index already exists then returns false
             */
            bool AddNamedIndex(size_t inIndex, std::string inName);

            size_t GetMaxSupportedIndexes() { return m_MaxNmaedIndexes; }

            size_t GetNumberOfIndexes() { return m_NamedIndexes.size(); }

        protected:
            /**
             * The max number of contexes we support in a snapshot
             */
            size_t m_MaxNmaedIndexes;;

            std::map<size_t, std::string> m_NamedIndexes;
        };
    }

    template <>
    struct Serialization::TypeSerializer<Containers::NamedIndexes>
    {
        static bool SerializeRead(Serialization::ReadBuffer &inBuffer, Containers::NamedIndexes &inValue);
        static bool SerializeWrite(Serialization::WriteBuffer &inBuffer, const Containers::NamedIndexes &inValue);
    };
}

#endif //__NAMED_INDEXES_H__