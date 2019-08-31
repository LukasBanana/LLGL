/*
 * DbgGraphicsPipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_GRAPHICS_PIPELINE_H
#define LLGL_DBG_GRAPHICS_PIPELINE_H


#include <LLGL/GraphicsPipeline.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <string>


namespace LLGL
{


class DbgGraphicsPipeline final : public GraphicsPipeline
{

    public:

        void SetName(const char* name) override;

    public:

        DbgGraphicsPipeline(GraphicsPipeline& instance, const GraphicsPipelineDescriptor& desc);

    public:

        GraphicsPipeline&                   instance;
        const GraphicsPipelineDescriptor    desc;
        std::string                         label;

};


} // /namespace LLGL


#endif



// ================================================================================
