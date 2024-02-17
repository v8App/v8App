// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __BINARY_FILE_H__
#define __BINARY_FILE_H__

#include "BaseAsset.h"

namespace v8App
{
    namespace Assets
    {
        class BinaryAsset : public BaseAsset
        {
        public:
            BinaryAsset(std::filesystem::path inAssetPath) : BaseAsset(inAssetPath){}
            virtual ~BinaryAsset(){}

            virtual bool ReadAsset() override;
            virtual bool WriteAsset() override;

            const std::vector<uint8_t> GetContent() { return m_Content; };
            bool SetContent(const std::vector<uint8_t>& inContents);

        protected:
            std::vector<uint8_t> m_Content;
        };
    }
}
#endif //__BINARY_FILE_H__