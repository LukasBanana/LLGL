/*
 * RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_H__
#define __LLGL_RENDER_CONTEXT_H__


#include "Export.h"
#include <string>


namespace LLGL
{


struct RenderContextDescriptor
{
    int colorDepth = 32;    //!< Color bit-depth. By default 32.
};


//! Render context interface.
class LLGL_EXPORT RenderContext
{

    public:

        /* ----- Render system ----- */

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator = (const RenderContext&) = delete;

        virtual ~RenderContext();

        //! Returns a descriptive version string of this render context (e.g. "OpenGL 4.5").
        virtual std::string GetVersion() const = 0;

    protected:

        RenderContext() = default;

    private:

};


} // /namespace LLGL


#endif



// ================================================================================
