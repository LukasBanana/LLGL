/*
 * MTPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_PIPELINE_LAYOUT_H
#define LLGL_MT_PIPELINE_LAYOUT_H


#import <MetalKit/MetalKit.h>

#include "../Shader/MTShaderStage.h"
#include "MTDescriptorCache.h"
#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <vector>


namespace LLGL
{


class MTPipelineLayout final : public PipelineLayout
{

    public:

        std::uint32_t GetNumHeapBindings() const override;
        std::uint32_t GetNumBindings() const override;
        std::uint32_t GetNumStaticSamplers() const override;
        std::uint32_t GetNumUniforms() const override;

    public:

        MTPipelineLayout(id<MTLDevice> device, const PipelineLayoutDescriptor& desc);
        ~MTPipelineLayout();

        void SetStaticVertexSamplers(id<MTLRenderCommandEncoder> renderEncoder) const;
        void SetStaticFragmentSamplers(id<MTLRenderCommandEncoder> renderEncoder) const;
        void SetStaticKernelSamplers(id<MTLComputeCommandEncoder> computeEncoder) const;

        inline const std::vector<BindingDescriptor>& GetHeapBindings() const
        {
            return heapBindings_;
        }

        inline const std::vector<MTDynamicResourceLayout>& GetDynamicBindings() const
        {
            return dynamicBindings_;
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

        std::vector<BindingDescriptor>          heapBindings_;
        std::vector<MTDynamicResourceLayout>    dynamicBindings_;

        std::vector<id<MTLSamplerState>>        staticSamplerStates_;
        std::vector<NSUInteger>                 staticSamplerIndices_;
        std::uint32_t                           numStaticSamplerPerStage_[MTShaderStage_Count];
        std::uint32_t                           numStaticSamplers_                              = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
