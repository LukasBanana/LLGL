/*
 * HelperMacros.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_HELPER_MACROS_H
#define LLGL_HELPER_MACROS_H


#define LLGL_COMPARE_MEMBER_EQ(MEMBER) \
    (lhs.MEMBER == rhs.MEMBER)

#define LLGL_COMPARE_MEMBER_SWO(MEMBER)         \
    if (lhs.MEMBER < rhs.MEMBER) return true;   \
    if (lhs.MEMBER > rhs.MEMBER) return false

#define LLGL_COMPARE_MEMBER_SWO_LAST(MEMBER) \
    return (lhs.MEMBER < rhs.MEMBER)

#define LLGL_CASE_TO_STR(VALUE) \
    case VALUE: return #VALUE


#endif



// ================================================================================
