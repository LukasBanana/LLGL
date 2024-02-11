/*
 * DbgPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        void SetDebugName(const char* name) override;

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
