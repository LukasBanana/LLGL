/*
 * MTPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_PIPELINE_LAYOUT_H
#define LLGL_MT_PIPELINE_LAYOUT_H


#import <MetalKit/MetalKit.h>

#include "../Shader/MTShaderStage.h"
#include "MTDescriptorCache.h"
#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/Vector.h>


namespace LLGL
{


class MTPipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        MTPipelineLayout(id<MTLDevice> device, const PipelineLayoutDescriptor& desc);
        ~MTPipelineLayout();

        void SetStaticVertexSamplers(id<MTLRenderCommandEncoder> renderEncoder) const;
        void SetStaticFragmentSamplers(id<MTLRenderCommandEncoder> renderEncoder) const;
        void SetStaticKernelSamplers(id<MTLComputeCommandEncoder> computeEncoder) const;

        inline const vector<BindingDescriptor>& GetHeapBindings() const
        {
            return heapBindings_;
        }

        inline const vector<MTDynamicResourceLayout>& GetDynamicBindings() const
        {
            return dynamicBindings_;
        }

        inline const vector<UniformDescriptor>& GetUniforms() const
        {
            return uniforms_;
        }

    private:

        void BuildDynamicBindings(const ArrayView<BindingDescriptor>& bindings);

        void BuildStaticSamplers(
            id<MTLDevice>                               device,
            const ArrayView<StaticSamplerDescriptor>&   staticSamplerDescs
        );

        std::size_t BuildStaticSamplersForStage(
            id<MTLDevice>                               device,
            const ArrayView<StaticSamplerDescriptor>&   staticSamplerDescs,
            long                                        stageFlags
        );

    private:

        vector<BindingDescriptor>          heapBindings_;
        vector<MTDynamicResourceLayout>    dynamicBindings_;
        vector<UniformDescriptor>          uniforms_;

        vector<id<MTLSamplerState>>        staticSamplerStates_;
        vector<NSUInteger>                 staticSamplerIndices_;
        std::uint32_t                           numStaticSamplerPerStage_[MTShaderStage_Count];
        std::uint32_t                           numStaticSamplers_                              = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
