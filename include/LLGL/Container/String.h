/*
 * String.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STRING_H
#define LLGL_STRING_H

#ifndef LLGL_REPLACE_STD_STRING
#include <string>
#endif

#ifdef LLGL_REPLACE_STD_STRING
typedef LLGL_CUSTOM_STRING  string;
typedef LLGL_CUSTOM_WSTRING wstring;
#else
typedef std::string  string;
typedef std::wstring wstring;
#endif

#endif

// ================================================================================
