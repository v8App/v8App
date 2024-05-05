// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __BINARY_FILE_H__
#define __BINARY_FILE_H__

#include <vector>

#include "BaseAsset.h"
namespace v8App
{
    namespace Assets
    {
        using BinaryByteVector = std::vector<uint8_t>;

        class BinaryAsset : public BaseAsset
        {
        public:
            BinaryAsset(std::filesystem::path inAssetPath) : BaseAsset(inAssetPath){}
            virtual ~BinaryAsset(){}

            virtual bool ReadAsset() override;
            virtual bool WriteAsset() override;

            const BinaryByteVector GetContent() { return m_Content; };
            bool SetContent(const BinaryByteVector& inContents);

        protected:
            BinaryByteVector m_Content;
        };
    }
}
#endif //__BINARY_FILE_H__