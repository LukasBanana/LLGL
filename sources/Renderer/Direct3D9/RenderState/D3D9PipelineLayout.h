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
#include <LLGL/Container/StringView.h>
#include <string>
#include "../Texture/D3D9EmulatedSampler.h"
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9StateManager;

struct D3D9ResourceBinding
{
    ResourceType    type;
    UINT            combiners; // If `combiners` is greater than zero, `stage` is interpreted as index into the array of combined sampler stages.
    DWORD           stage;
};

struct D3D9ResourceBindingTable
{
    std::vector<D3D9ResourceBinding> resourceBindings;
};

class D3D9PipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9PipelineLayout(const PipelineLayoutDescriptor& desc);

        void BindStaticSamplers(D3D9StateManager& stateMngr) const;

        inline const std::vector<UniformDescriptor>& GetUniforms() const
        {
            return uniformDesc_;
        }

        inline const D3D9ResourceBindingTable& GetResourceBindingTable() const
        {
            return resourceBindingTable_;
        }

        inline const std::vector<DWORD>& GetCombinedSamplerStages() const
        {
            return combinedSamplerStages_;
        }

    private:

        struct D3DStaticSamplerAndStage
        {
            DWORD                   stage;
            D3D9EmulatedSamplerSPtr sampler;
        };

    private:

        void BuildCombinedSamplerStages(const PipelineLayoutDescriptor& pipelineLayoutDesc, D3D9ResourceBinding& resourceBinding, StringView name);
        void BuildStaticSampler(const PipelineLayoutDescriptor& pipelineLayoutDesc, const StaticSamplerDescriptor& staticSamplerDesc);
        bool AddCombinedStaticSamplers(const PipelineLayoutDescriptor& pipelineLayoutDesc, const StaticSamplerDescriptor& staticSamplerDesc, const D3D9EmulatedSamplerSPtr& newStaticSampler);
        void AddStaticSampler(const BindingSlot& slot, const D3D9EmulatedSamplerSPtr& newStaticSampler);

    private:

        std::vector<UniformDescriptor>          uniformDesc_;
        D3D9ResourceBindingTable                resourceBindingTable_;
        std::vector<DWORD>                      combinedSamplerStages_;
        std::vector<D3DStaticSamplerAndStage>   staticSamplers_;                // Dupliated for combined sampler stages
        std::uint32_t                           numUniqueStaticSampler_ = 0;    // Number of unique static samplers

};


} // /namespace LLGL


#endif



// ================================================================================
