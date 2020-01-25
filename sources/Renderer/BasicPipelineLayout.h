/*
 * BasicPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BASIC_PIPELINE_LAYOUT_H
#define LLGL_BASIC_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <vector>


namespace LLGL
{


// This class only holds a copy of the binding descriptor list.
class LLGL_EXPORT BasicPipelineLayout : public PipelineLayout
{

    public:

        std::uint32_t GetNumBindings() const override;

    public:

        BasicPipelineLayout(const PipelineLayoutDescriptor& desc);

        // Returns the copied list of binding descriptors.
        inline const std::vector<BindingDescriptor>& GetBindings() const
        {
            return bindings_;
        }

    private:

        std::vector<BindingDescriptor> bindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
