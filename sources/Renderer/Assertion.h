/*
 * Assertion.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_ASSERTION_H__
#define __LLGL_ASSERTION_H__


#include <stdexcept>


namespace LLGL
{


#define LLGL_ASSERT_PTR(PTR)                                                    \
    if (!PTR)                                                                   \
    {                                                                           \
        throw std::invalid_argument(                                            \
            __FUNCTION__ ": null pointer exception of parameter \"" #PTR "\""   \
        );                                                                      \
    }


} // /namespace LLGL


#endif



// ================================================================================
