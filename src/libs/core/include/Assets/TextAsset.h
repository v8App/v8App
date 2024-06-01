// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __TEXT_ASSET_H__
#define __TEXT_ASSET_H__

#include "BaseAsset.h"

namespace v8App
{
    namespace Assets
    {
        /**
         * Text Asset override this to load specifc text formats
        */
        class TextAsset : public BaseAsset
        {
        public:
            TextAsset(std::filesystem::path inAssetPath) : BaseAsset(inAssetPath) {}
            virtual ~TextAsset() {}

            virtual bool ReadAsset() override;
            virtual bool WriteAsset() override;

            const std::string &GetContent() { return m_Contents; };
            bool SetContent(const std::string &inContents);

        protected:
            std::string m_Contents;
        };
    }
}

#endif //__TEXT_ASSET_H__