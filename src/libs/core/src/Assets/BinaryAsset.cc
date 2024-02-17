// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Assets/BinaryAsset.h"
#include "Logging/Log.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"

namespace v8App
{
    namespace Assets
    {
        bool BinaryAsset::ReadAsset()
        {
            std::ifstream file(m_AssetPath, std::ios::in | std::ios::binary);
            if (file.is_open() == false || file.bad())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open binary asset: {} for reading", m_AssetPath)}};
                LOG_ERROR(msg);
                return false;
            }

            file.unsetf(std::ios::skipws);

            file.seekg(0, std::ios::end);
            size_t length = file.tellg();
            file.seekg(0, std::ios::beg);

            if (m_Content.size() < length)
            {
                m_Content.reserve(length);
            }
            m_Content.resize(0);

            m_Content.insert(m_Content.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());
            m_Content.shrink_to_fit();

            return m_Content.size() == length;
        }

        bool BinaryAsset::WriteAsset()
        {
            std::ofstream file(m_AssetPath, std::ios::out | std::ios::binary);
            if (file.is_open() == false || file.bad())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open binary asset: {} for writing", m_AssetPath)}};
                LOG_ERROR(msg);
                return false;
            }

            file.write((const char *)m_Content.data(), m_Content.size());

            return file.tellp() == m_Content.size();
        }

        bool BinaryAsset::SetContent(const std::vector<uint8_t> &inContents)
        {
            m_Content = inContents;
            return true;
        }

    }
}
