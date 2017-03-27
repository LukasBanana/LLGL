/*
 * DbgGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_GRAPHICS_PIPELINE_H
#define LLGL_DBG_GRAPHICS_PIPELINE_H


#include <LLGL/GraphicsPipeline.h>


namespace LLGL
{


class DbgGraphicsPipeline : public GraphicsPipeline
{

    public:

        DbgGraphicsPipeline(GraphicsPipeline& instance, const GraphicsPipelineDescriptor& desc) :
            instance( instance ),
            desc    ( desc     )
        {
        }

        GraphicsPipeline&                   instance;
        const GraphicsPipelineDescriptor    desc;

};


} // /namespace LLGL


#endif



// ================================================================================
