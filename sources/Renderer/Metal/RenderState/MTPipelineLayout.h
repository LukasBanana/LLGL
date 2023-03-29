/*
 * MTPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_PIPELINE_LAYOUT_H
#define LLGL_MT_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
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

        MTPipelineLayout(const PipelineLayoutDescriptor& desc);

        inline const std::vector<BindingDescriptor>& GetHeapBindings() const
        {
            return heapBindings_;
        }

    private:

        std::vector<BindingDescriptor> heapBindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
