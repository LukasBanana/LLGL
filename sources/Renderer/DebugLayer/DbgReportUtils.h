/*
 * DbgReportUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_REPORT_UTILS_H
#define LLGL_DBG_REPORT_UTILS_H


#include <cstdint>


namespace LLGL
{


inline const char* ToByteLabel(std::uint64_t n)
{
    return (n == 1 ? "byte" : "bytes");
}

inline const char* ToVertexLabel(std::uint64_t n)
{
    return (n == 1 ? "vertex" : "vertices");
}


} // /namespace LLGL


#endif



// ================================================================================
