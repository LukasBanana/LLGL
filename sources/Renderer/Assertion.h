/*
 * Assertion.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_ASSERTION_H__
#define __LLGL_ASSERTION_H__


#include <stdexcept>
#include <string>


namespace LLGL
{


#ifdef _MSC_VER
#   define LLGL_ASSERT_INFO(INFO) (__FUNCTION__ ": " INFO)
#else
#   define LLGL_ASSERT_INFO(INFO) (std::string(__FUNCTION__) + std::string(": ") + std::string(INFO))
#endif

#define LLGL_ASSERT_PTR(PTR)                                                \
    if (!PTR)                                                               \
    {                                                                       \
        throw std::invalid_argument(                                        \
            std::string(__FUNCTION__) +                                     \
                std::string(": null pointer exception of parameter \"") +   \
                std::string(#PTR) + std::string("\"")                       \
        );                                                                  \
    }


} // /namespace LLGL


#endif



// ================================================================================
