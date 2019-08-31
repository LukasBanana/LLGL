/*
 * Assertion.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ASSERTION_H
#define LLGL_ASSERTION_H


#include "Exception.h"
#include <stdexcept>
#include <string>


namespace LLGL
{


#ifdef _MSC_VER
#   define LLGL_ASSERT_INFO(INFO) (__FUNCTION__ ": " INFO)
#else
#   define LLGL_ASSERT_INFO(INFO) (std::string(__FUNCTION__) + std::string(": ") + std::string(INFO))
#endif

#define LLGL_ASSERT_PTR(PARAM)                          \
    if (!(PARAM))                                       \
        ThrowNullPointerExcept(__FUNCTION__, #PARAM)

#define LLGL_ASSERT_UPPER_BOUND(PARAM, UPPER_BOUND)                                                                 \
    if ((PARAM) >= (UPPER_BOUND))                                                                                   \
        ThrowExceededUpperBoundExcept(__FUNCTION__, #PARAM, static_cast<int>(PARAM), static_cast<int>(UPPER_BOUND))

#define LLGL_ASSERT_RANGE(PARAM, MAXIMUM)                                                                       \
    if ((PARAM) > (MAXIMUM))                                                                                    \
        ThrowExceededMaximumExcept(__FUNCTION__, #PARAM, static_cast<int>(PARAM), static_cast<int>(MAXIMUM))

#define LLGL_ASSERT_FEATURE_SUPPORT(FEATURE)                            \
    if (!GetRenderingCaps().features.FEATURE)                           \
        ThrowRenderingFeatureNotSupportedExcept(__FUNCTION__, #FEATURE)


} // /namespace LLGL


#endif



// ================================================================================
