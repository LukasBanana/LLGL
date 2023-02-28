/*
 * NullPipelineLayout.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_PIPELINE_LAYOUT_H
#define LLGL_NULL_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <string>


namespace LLGL
{


class NullPipelineLayout final : public PipelineLayout
{

    public:

        void SetName(const char* name) override;

        std::uint32_t GetNumHeapBindings() const override;
        std::uint32_t GetNumBindings() const override;
        std::uint32_t GetNumStaticSamplers() const override;
        std::uint32_t GetNumUniforms() const override;

    public:

        NullPipelineLayout(const PipelineLayoutDescriptor& desc);

    public:

        const PipelineLayoutDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
