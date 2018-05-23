/*
 * RenderSystemChild.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_CHILD_H
#define LLGL_RENDER_SYSTEM_CHILD_H


#include "Export.h"


namespace LLGL
{


/**
\brief Base class for all interfaces whoes instances are owned by the RenderSystem.
\remarks Sub classes of this interface cannot be copied on its own, since its copy constructor and copy operator are deleted functions.
*/
class LLGL_EXPORT RenderSystemChild
{

    public:

        RenderSystemChild(const RenderSystemChild&) = delete;
        RenderSystemChild& operator = (const RenderSystemChild&) = delete;

        virtual ~RenderSystemChild() = default;

    protected:

        RenderSystemChild() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
