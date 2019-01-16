/*
 * RenderSystemChild.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_CHILD_H
#define LLGL_RENDER_SYSTEM_CHILD_H


#include "NonCopyable.h"


namespace LLGL
{


//! Base class for all interfaces whoes instances are owned by the RenderSystem.
class LLGL_EXPORT RenderSystemChild : public NonCopyable { };

#if 0
class LLGL_EXPORT RenderSystemChild : public NonCopyable
{

    public:

        #if 0//TODO
        //! Returns the interface enumeration entry this class instance is associated with.
        virtual Interface QueryInterface() const = 0;
        #endif

        /**
        \brief Sets the name of this class instance for debugging purposes.
        \param[in] name Pointer to a null terminated string that specifies the name. Must not be null!
        \note Only supported in debug mode or when the debug layer is enabled. Otherwise, the function has no effect.
        \see CommandBuffer::PushDebugGroup
        */
        virtual void SetDebugName(const char* name) = 0;

};
#endif


} // /namespace LLGL


#endif



// ================================================================================
