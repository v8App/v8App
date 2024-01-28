// Copyright 2020 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <fstream>

#include "Assets/TextAsset.h"
#include "Logging/Log.h"
#include "Logging/LogMacros.h"
#include "Utils/Format.h"

namespace v8App
{
    namespace Assets
    {
        bool TextAsset::ReadAsset()
        {
            std::ifstream file(m_AssetPath, std::ios::in);
            if (file.is_open() == false || file.bad())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open text asset: {} for reading", m_AssetPath)}};
                LOG_ERROR(msg);
                return false;
            }

            std::ostringstream stream;
            stream << file.rdbuf();
            m_Contents = stream.str();
            return true;
        }

        bool TextAsset::WriteAsset()
        {
            std::ofstream file(m_AssetPath, std::ios::out);
            if (file.is_open() == false || file.bad())
            {
                Log::LogMessage msg = {
                    {Log::MsgKey::Msg, Utils::format("Failed to open text asset: {} for writing", m_AssetPath)}};
                LOG_ERROR(msg);
                return false;
            }

            file.write(m_Contents.c_str(), m_Contents.length());

            return m_Contents.length() == file.tellp();
        }

        bool TextAsset::SetContent(const std::string &inContent)
        {
            m_Contents = inContent;
            return true;
        }
    }
}
