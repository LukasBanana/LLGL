/*
 * Path.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Path.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/Strings.h>


namespace LLGL
{

namespace Path
{


static bool IsCharAnySeparator(char c)
{
    return (c == '/' || c == '\\');
}

LLGL_EXPORT UTF8String Sanitize(const UTF8String& path)
{
    std::string s(path.begin(), path.end());

    /* Sanitize separators and remove redundant upper-level directory entries */
    auto JoinSubPaths = [](std::string& s, std::size_t off) -> std::size_t
    {
        /* Find start of previous path */
        for (std::size_t i = off; i-- > 0;)
        {
            if (IsCharAnySeparator(s[i]))
            {
                s = s.substr(0, i) + s.substr(off);
                return i;
            }
        }
        return off;
    };

    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (IsCharAnySeparator(s[i]))
        {
            s[i] = GetSeparator();
            if (i >= 2 && s[i - 1] == '.' && s[i - 2] == '.')
                i = JoinSubPaths(s, i);
        }
    }

    /* Strip trailing separator */
    if (!s.empty() && IsCharAnySeparator(s.back()))
        s.pop_back();

    return s;
}

LLGL_EXPORT UTF8String Combine(const UTF8String& lhs, const UTF8String& rhs)
{
    if (lhs.empty() && rhs.empty())
        return "";
    if (lhs.empty())
        return Sanitize(rhs);
    if (rhs.empty())
        return Sanitize(lhs);
    char sep[2] = { GetSeparator(), '\0' };
    return Sanitize(Sanitize(lhs) + UTF8String{ sep } + Sanitize(rhs));
}


} // /nameapace Path

} // /namespace LLGL



// ================================================================================
