/*
 * MTRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderContext.h"


namespace LLGL
{


/* ----- Common ----- */

MTRenderContext::MTRenderContext(
    RenderContextDescriptor         desc,
    const std::shared_ptr<Surface>& surface)
{
    SetOrCreateSurface(surface, desc.videoMode, nullptr);
    desc.videoMode = GetVideoMode();

    //TODO: create swap-chain
}

MTRenderContext::~MTRenderContext()
{
}

void MTRenderContext::Present()
{
    //todo
}


/*
 * ======= Private: =======
 */


} // /namespace LLGL



// ================================================================================
