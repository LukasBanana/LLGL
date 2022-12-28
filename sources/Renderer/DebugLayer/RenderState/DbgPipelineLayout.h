/*
 * DbgPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_PIPELINE_LAYOUT_H
#define LLGL_DBG_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <string>


namespace LLGL
{


class DbgPipelineLayout final : public PipelineLayout
{

    public:

        void SetName(const char* name) override;

        std::uint32_t GetNumBindings() const override;

    public:

        DbgPipelineLayout(PipelineLayout& instance, const PipelineLayoutDescriptor& desc);

    public:

        PipelineLayout&                 instance;
        const PipelineLayoutDescriptor  desc;
        std::string                     label;

};


} // /namespace LLGL


#endif



// ================================================================================
