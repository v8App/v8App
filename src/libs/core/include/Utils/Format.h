// Copyright 2020 - 2024 The v8App Authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <sstream>
#include <string_view>

namespace v8App
{
    namespace Utils
    {
        // Because Apple's clang doesn't support std::format at the moment
        template <typename T>
        inline void format_helper(std::ostringstream &oss, std::string_view &str, const T &value)
        {
            bool found = false;
            std::size_t leftBracket = 0;
            std::size_t rigthBracket = std::string::npos;
            do
            {
                leftBracket = str.find('{', leftBracket);
                if (leftBracket == str.npos)
                {
                    return;
                }
                rigthBracket = str.find('}', leftBracket + 1);
                if (rigthBracket == str.npos)
                {
                    return;
                }
                //for a replace it has to be {} no spaces or anything else in between
                if (leftBracket + 1 == rigthBracket)
                {
                    found = true;
                    oss << str.substr(0, leftBracket) << value;
                    str = str.substr(rigthBracket + 1);
                }
                leftBracket++;
            } while (found == false);
        }

        template <typename... TArgs>
        inline std::string format(std::string_view str, TArgs... args)
        {
            std::ostringstream oss;
            (format_helper(oss, str, args), ...);
            oss << str;
            return oss.str();
        }
    }
}

#endif //__FORMAT_H__