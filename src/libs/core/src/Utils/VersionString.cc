// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <vector>
#include <regex>

#include "Utils/VersionString.h"

namespace v8App
{
    namespace Utils
    {
        VersionString::VersionString(std::string inVersion)
        {
            SetVersionString(inVersion);
        }

        int VersionString::CompareVersions(const VersionString &inVersion) const
        {
            if (IsVersionString() == false)
            {
                return -1;
            }
            if (inVersion.IsVersionString() == false)
            {
                return 1;
            }
            if (m_Major < inVersion.m_Major)
            {
                return -1;
            }
            if (m_Major > inVersion.m_Major)
            {
                return 1;
            }
            if (m_Minor < inVersion.m_Minor)
            {
                return -1;
            }
            if (m_Minor > inVersion.m_Minor)
            {
                return 1;
            }
            if (m_Patch < inVersion.m_Patch)
            {
                return -1;
            }
            if (m_Patch > inVersion.m_Patch)
            {
                return 1;
            }
            if (m_PreRelease.length() && inVersion.m_PreRelease.empty())
            {
                return -1;
            }
            if (m_PreRelease.empty() && inVersion.m_PreRelease.length())
            {
                return 1;
            }
            std::regex delimter("\\.");
            std::vector<std::string> preRelease(std::sregex_token_iterator(m_PreRelease.begin(), m_PreRelease.end(), delimter, -1), {});
            std::vector<std::string> inPreRelease(std::sregex_token_iterator(inVersion.m_PreRelease.begin(), inVersion.m_PreRelease.end(), delimter, -1), {});
            size_t length = preRelease.size() < inPreRelease.size() ? preRelease.size() : inPreRelease.size();

            for (int i = 0; i < length; i++)
            {
                const std::string left = preRelease.at(i);
                const std::string right = inPreRelease.at(i);

                bool leftIsNumber = IsNumber(left);
                bool rightIsNumber = IsNumber(right);

                if (leftIsNumber && rightIsNumber)
                {
                    int iLeft = ConvertStringToInt(left);
                    int iRight = ConvertStringToInt(right);
                    if (iLeft < iRight)
                    {
                        return -1;
                    }
                    if (iLeft > iRight)
                    {
                        return 1;
                    }
                }
                else if (leftIsNumber && rightIsNumber == false)
                {
                    return -1;
                }
                else if (leftIsNumber == false && rightIsNumber)
                {
                    return 1;
                }
                int result = left.compare(right);
                if (result != 0)
                {
                    return result < 0 ? -1 : 1;
                }
            }
            if (preRelease.size() == inPreRelease.size())
            {
                return 0;
            }
            return preRelease.size() < inPreRelease.size() ? -1 : 1;
        }

        void VersionString::ParseVersionString()
        {
            std::regex semVer("^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)(?:-((?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$");
            std::smatch matches;

            if (std::regex_match(m_Version, matches, semVer) == false)
            {
                m_Valid = false;
                return;
            }
            m_Major = ConvertStringToInt(matches[1].str());
            m_Minor = ConvertStringToInt(matches[2].str());
            m_Patch = ConvertStringToInt(matches[3].str());
            m_PreRelease = matches[4].str();
            m_Build = matches[5].str();
            m_Valid = true;
        }

        int VersionString::ConvertStringToInt(const std::string &inInt) const
        {
            // empty strings are 0;
            if (inInt.empty())
            {
                return 0;
            }
            try
            {
                return std::stoi(inInt);
            }
            catch (std::invalid_argument const &e)
            {
            }
            catch (std::out_of_range const &e)
            {
            }
            // we just fell through for the errors since they do the same thing and any others will escape
            //  TODO: Log the nummber
            return -1;
        }

        bool VersionString::IsNumber(const std::string &inValue) const
        {
            return inValue.empty() == false && std::find_if(inValue.begin(), inValue.end(),
                                                            [](unsigned char c)
                                                            { return std::isdigit(c) == false; }) == inValue.end();
        }

        bool VersionString::SetVersionString(std::string inVersion)
        {
            m_Version = inVersion;
            ParseVersionString();
            return IsVersionString();
        }

        std::string VersionString::GetVersionString()
        {
            if (m_Version == "")
            {
                if (m_Major > -1)
                {
                    m_Version += std::to_string(m_Major);
                }
                else
                {
                    m_Version += "0";
                }

                m_Version += ".";
                if (m_Minor > -1)
                {
                    m_Version += std::to_string(m_Minor);
                }
                else
                {
                    m_Version += "0";
                }

                m_Version += ".";
                if (m_Patch > -1)
                {
                    m_Version += std::to_string(m_Patch);
                }
                else
                {
                    m_Version += "0";
                }

                if (m_PreRelease != "")
                {
                    m_Version += "-" + m_PreRelease;
                }

                if (m_Build != "")
                {
                    m_Version += "+" + m_Build;
                }
            }
            return m_Version;
        }
    }

    bool Serialization::TypeSerializer<Utils::VersionString>::Serialize(Serialization::BaseBuffer &inBuffer, Utils::VersionString &inValue)
    {
        if (inBuffer.IsReader())
        {
            std::string version;
            inBuffer >> version;
            if (inBuffer.HasErrored())
            {
                return false;
            }
            inValue.SetVersionString(version);
        }
        else
        {
            inBuffer << inValue.GetVersionString();
        }
        return true;
    }
}