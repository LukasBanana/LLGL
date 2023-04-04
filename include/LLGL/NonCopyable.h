/*
 * NonCopyable.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NON_COPYABLE_H
#define LLGL_NON_COPYABLE_H


#include <LLGL/Export.h>


namespace LLGL
{


/**
\brief Base class for all classes with virtual function tables in LLGL.
\remarks Sub classes of this interface cannot be copied on its own, since its copy constructor and copy operator are deleted functions.
\see Interface
*/
class LLGL_EXPORT NonCopyable
{

    public:

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator = (const NonCopyable&) = delete;

        virtual ~NonCopyable() = default;

    protected:

        NonCopyable() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
