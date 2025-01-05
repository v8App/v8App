// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __VERSIONS_H__
#define __VERSIONS_H__

#include <string>

#include "Serialization/TypeSerializer.h"

namespace v8App
{
    namespace Utils
    {
        /**
         * Implements class that deals with version string parsing based on Semantic version strings
         * See https://semver.org/
         */
        class VersionString
        {
        public:
            VersionString() = default;
            VersionString(std::string inVersion);
            ~VersionString() = default;

            bool IsVersionString() const { return m_Valid; }

            int GetMajor() const { return m_Major; }
            int GetMinor() const { return m_Minor; }
            int GetPatch() const { return m_Patch; }
            const std::string &GetPreRelease() const { return m_PreRelease; }
            const std::string &GetBuild() const { return m_Build; }

            void SetMajor(int inMajor)
            {
                m_Major = inMajor;
                GenerateVersionString();
            }
            void SetMinor(int inMinor)
            {
                m_Minor = inMinor;
                GenerateVersionString();
            }
            void SetPatch(int inPatch)
            {
                m_Patch = inPatch;
                GenerateVersionString();
            }
            void SetPreRelease(std::string inPreRelease)
            {
                m_PreRelease = inPreRelease;
                GenerateVersionString();
            }
            void SetBuild(std::string inBuild)
            {
                m_Build = inBuild;
                GenerateVersionString();
            }

            bool SetVersionString(std::string inVersion);

            int CompareVersions(const VersionString &inVersion) const;
            auto operator<=>(const VersionString &inVersion) const { return CompareVersions(inVersion); }
            auto operator<=>(const std::string &inVersion) const { return CompareVersions(VersionString(inVersion)); }
            bool operator==(const VersionString &inVersion) const { return CompareVersions(inVersion) == 0; }
            bool operator==(const std::string inVersion) const { return CompareVersions(VersionString(inVersion)) == 0; }

            /**
             * Returns the already built version string or if empty tries and build it.
             * If Major, Minor or Patch are -1 then an empty string is returned
             */
            std::string GetVersionString() const;

        private:
            void ParseVersionString();
            int ConvertStringToInt(const std::string &inInt) const;
            bool IsNumber(const std::string &inValue) const;

            void GenerateVersionString();

            std::string m_Version;
            bool m_Valid{false};
            int m_Major{-1};
            int m_Minor{-1};
            int m_Patch{-1};
            std::string m_PreRelease;
            std::string m_Build;
        };

    }

    template <>
    struct Serialization::TypeSerializer<Utils::VersionString>
    {
        static bool SerializeRead(Serialization::ReadBuffer &inBuffer, Utils::VersionString &inValue);
        static bool SerializeWrite(Serialization::WriteBuffer &inBuffer, const Utils::VersionString &inValue);
    };
}
#endif //__VERSIONS_H__