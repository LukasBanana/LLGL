/*
 * D3D9PipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_PIPELINE_LAYOUT_H
#define LLGL_D3D9_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <string>


namespace LLGL
{


class D3D9PipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9PipelineLayout(const PipelineLayoutDescriptor& desc);

        inline const std::vector<UniformDescriptor>& GetUniforms() const
        {
            return uniformDesc_;
        }

    private:

        std::uint32_t                   numHeapBindings_      = 0;
        std::uint32_t                   numBindings_          = 0;
        std::uint32_t                   numStaticSamplers_    = 0;
        std::vector<UniformDescriptor>  uniformDesc_;

};


} // /namespace LLGL


#endif



// ================================================================================
