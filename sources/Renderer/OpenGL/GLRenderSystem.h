/*
 * GLRenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_SYSTEM_H__
#define __LLGL_GL_RENDER_SYSTEM_H__


#include <LLGL/RenderSystem.h>
#include "GLRenderContext.h"
#include <string>
#include <memory>
#include <vector>


namespace LLGL
{


class GLRenderSystem : public RenderSystem
{

    public:

        /* ----- Render system ----- */

        GLRenderSystem();
        ~GLRenderSystem();

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

    private:

        std::vector<std::unique_ptr<GLRenderContext>> renderContexts_;

};


} // /namespace LLGL


#endif



// ================================================================================
