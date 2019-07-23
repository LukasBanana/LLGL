/*
 * GraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GRAPHICS_PIPELINE_H
#define LLGL_GRAPHICS_PIPELINE_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\brief Graphics pipeline interface.
\see RenderSystem::CreateGraphicsPipeline
\see CommandBuffer::SetGraphicsPipeline
*/
class LLGL_EXPORT GraphicsPipeline : public RenderSystemChild
{
    LLGL_DECLARE_INTERFACE( InterfaceID::GraphicsPipeline );
};


} // /namespace LLGL


#endif



// ================================================================================
