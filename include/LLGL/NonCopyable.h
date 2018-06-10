/*
 * NonCopyable.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NON_COPYABLE_H
#define LLGL_NON_COPYABLE_H


#include "Export.h"


namespace LLGL
{


/**
\brief Base class for all interfaces in LLGL.
\remarks Sub classes of this interface cannot be copied on its own, since its copy constructor and copy operator are deleted functions.
\see Display
\see RenderSystem
\see RenderSystemChild
\see ShaderUniform
\see Surface
\see Timer
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
