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

//TODO: maybe add "QueryInterfaceDesc" function, to determine class hierarchy from this base class
#if 0
class LLGL_EXPORT RenderSystemChild : public NonCopyable
{

    public:

        virtual bool QueryInterfaceDesc(InterfaceDescriptor& desc) const = 0;

};
#endif


} // /namespace LLGL


#endif



// ================================================================================
