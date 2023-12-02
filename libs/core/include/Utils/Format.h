// Copyright 2020 The v8App Authors. All rights reserved.
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
        //Because Apple's clang doesn't support std::format
        template<typename T>
        inline void format_helper(std::ostringstream& oss, std::string_view& str, const T& value)
        {
            std::size_t leftBracket = str.find('{');
            if(leftBracket == str.npos)
            {
                return;
            }
            std::size_t rigthBracket = str.find('}', leftBracket+1);
            if(rigthBracket == str.npos)
            {
                return;
            }
            oss << str.substr(0, leftBracket) << value;
            str = str.substr(rigthBracket+1);
        }

        template<typename... TArgs>
        inline std::string format(std::string_view str, TArgs...args)
        {
            std::ostringstream oss;
            (format_helper(oss, str, args), ...);
            oss << str;
            return oss.str();
        }
    }
}

#endif //__FORMAT_H__