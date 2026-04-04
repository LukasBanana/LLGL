/*
 * WGPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_LAYOUT_H
#define LLGL_WG_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>


namespace LLGL
{


class WGPipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        /* WGPipelineLayout(const PipelineLayoutDescriptor& desc); */

    private:

        // private fields ...

};


} // /namespace LLGL


#endif



// ================================================================================
