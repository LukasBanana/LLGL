/*
 * HelperMacros.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_HELPER_MACROS_H
#define LLGL_HELPER_MACROS_H


#define LLGL_COMPARE_MEMBER_EQ(MEMBER) \
    (lhs.MEMBER == rhs.MEMBER)

#define LLGL_COMPARE_MEMBER_SWO(MEMBER)         \
    if (lhs.MEMBER < rhs.MEMBER) { return -1; } \
    if (lhs.MEMBER > rhs.MEMBER) { return +1; }

#define LLGL_COMPARE_BOOL_MEMBER_SWO(MEMBER)        \
    if (!lhs.MEMBER &&  rhs.MEMBER) { return -1; }  \
    if ( lhs.MEMBER && !rhs.MEMBER) { return +1; }

#define LLGL_CASE_TO_STR(VALUE) \
    case VALUE: return #VALUE


#endif



// ================================================================================
