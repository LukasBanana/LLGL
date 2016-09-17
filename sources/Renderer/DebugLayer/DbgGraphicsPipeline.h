/*
 * DbgGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_GRAPHICS_PIPELINE_H__
#define __LLGL_DBG_GRAPHICS_PIPELINE_H__


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
