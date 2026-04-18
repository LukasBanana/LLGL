/*
 * WGPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_LAYOUT_H
#define LLGL_WG_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGPipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        WGPipelineLayout(WGPUDevice device, const PipelineLayoutDescriptor& desc);
        ~WGPipelineLayout();

        // Returns the native WebGPU pipeline layout object.
        inline WGPUPipelineLayout GetNative() const
        {
            return pipelineLayout_;
        }

    private:

        WGPUPipelineLayout  pipelineLayout_     = nullptr;
        WGPUBindGroupLayout bindGroupLayout_    = nullptr;
        std::uint32_t       numHeapBindings_    = 0;
        std::uint32_t       numBindings_        = 0;
        std::uint32_t       numStaticSamplers_  = 0;
        std::uint32_t       numUniforms_        = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
