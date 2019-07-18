/*
 * RenderSystemChild.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_CHILD_H
#define LLGL_RENDER_SYSTEM_CHILD_H


#include "Interface.h"


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
        Specifying a null pointer is equivalent of setting an empty string, effectively resetting the name to the default.
        \remarks This is used for debugging purposes only and the implementation is undefined,
        i.e. if the respective render system does not support debug labels this function call will be ignored silently.
        \remarks This function is especially useful in conjunction with diagnistoc tools (such as <a href="https://renderdoc.org">RenderDoc</a>) to better identify objects in the event history.
        \see CommandBuffer::PushDebugGroup
        \see CommandBuffer::PopDebugGroup
        \see RenderContextDescriptor::debugCallback
        */
        //virtual void SetName(const char* name) = 0;
        virtual void SetName(const char* name) {};

};


} // /namespace LLGL


#endif



// ================================================================================
