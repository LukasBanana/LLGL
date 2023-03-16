/*
 * D3D11PipelineLayout.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_PIPELINE_LAYOUT_H
#define LLGL_D3D11_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "../Texture/D3D11Sampler.h"
#include <d3d11.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D11StateManager;

enum D3DResourceType : std::uint32_t;

// Descriptor structure for D3D11 dynamic resource binding.
struct D3D11PipelineResourceBinding
{
    D3DResourceType type;
    UINT            slot;
    long            stageFlags;
};

class D3D11PipelineLayout final : public PipelineLayout
{

    public:

        std::uint32_t GetNumHeapBindings() const override;
        std::uint32_t GetNumBindings() const override;
        std::uint32_t GetNumStaticSamplers() const override;
        std::uint32_t GetNumUniforms() const override;

    public:

        D3D11PipelineLayout(ID3D11Device* device, const PipelineLayoutDescriptor& desc);

        void BindGraphicsStaticSamplers(D3D11StateManager& stateMngr) const;
        void BindComputeStaticSamplers(D3D11StateManager& stateMngr) const;

        // Returns the copied list of heap binding descriptors.
        inline const std::vector<BindingDescriptor>& GetHeapBindings() const
        {
            return heapBindings_;
        }

        // Returns the list of dynamic D3D resource bindings.
        inline const std::vector<D3D11PipelineResourceBinding>& GetBindings() const
        {
            return bindings_;
        }

        // Returns the list of uniform descritpors this pipeline layout was created with.
        inline const std::vector<UniformDescriptor>& GetUniforms() const
        {
            return uniforms_;
        }

    private:

        void BuildDynamicResourceBindings(const std::vector<BindingDescriptor>& bindingDescs);
        void BuildStaticSamplers(ID3D11Device* device, const std::vector<StaticSamplerDescriptor>& staticSamplerDescs);

    private:

        std::vector<BindingDescriptor>              heapBindings_;
        std::vector<D3D11PipelineResourceBinding>   bindings_;
        std::vector<D3D11StaticSampler>             staticSamplers_;
        std::vector<UniformDescriptor>              uniforms_;

};


} // /namespace LLGL


#endif



// ================================================================================
