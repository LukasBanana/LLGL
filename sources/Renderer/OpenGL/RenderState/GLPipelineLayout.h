/*
 * GLPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_PIPELINE_LAYOUT_H
#define LLGL_GL_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <vector>


namespace LLGL
{


// This class only holds a copy of the binding descriptor list.
class GLPipelineLayout : public PipelineLayout
{

    public:

        GLPipelineLayout(const PipelineLayoutDescriptor& desc);

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
