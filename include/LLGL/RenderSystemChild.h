/*
 * RenderSystemChild.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_CHILD_H
#define LLGL_RENDER_SYSTEM_CHILD_H


#include <LLGL/Interface.h>


namespace LLGL
{


//! Base class for all interfaces whoes instances are owned by the RenderSystem.
class LLGL_EXPORT RenderSystemChild : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::RenderSystemChild );

    public:

        /**
        \brief Sets the name of this class instance for debugging purposes.
        \param[in] name Pointer to a null terminated string that specifies the new name of this instance.
        Specifying a null pointer effectively removes the name from the object.
        The implementation of this function may alter the actual name depending on how many internal objects need to be labeled.
        \remarks This is used for debugging purposes only and the implementation is undefined,
        i.e. if the respective render system does not support debug labels this function call will be ignored silently.
        \remarks This function is especially useful in conjunction with diagnostic tools (such as <a href="https://renderdoc.org">RenderDoc</a>) to better identify objects in the event history.
        \remarks The default implementation has no effect.
        \see CommandBuffer::PushDebugGroup
        \see CommandBuffer::PopDebugGroup
        */
        virtual void SetName(const char* name);

};


} // /namespace LLGL


#endif



// ================================================================================
