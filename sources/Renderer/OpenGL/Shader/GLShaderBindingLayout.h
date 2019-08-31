/*
 * GLShaderBindingLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_BINDING_LAYOUT_H
#define LLGL_GL_SHADER_BINDING_LAYOUT_H


#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "../RenderState/GLPipelineLayout.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderBindingLayout;

using GLShaderBindingLayoutSPtr = std::shared_ptr<GLShaderBindingLayout>;

// Helper class to handle uniform block bindings and other resource bindings for GL shader programs with different pipeline layouts.
class GLShaderBindingLayout
{

    public:

        GLShaderBindingLayout() = default;
        GLShaderBindingLayout(const GLShaderBindingLayout&) = default;
        GLShaderBindingLayout& operator = (const GLShaderBindingLayout&) = default;

        GLShaderBindingLayout(const GLPipelineLayout& pipelineLayout);

        // Binds the resource slots to the specified GL shader program.
        void BindResourceSlots(GLuint program) const;

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        int CompareSWO(const GLShaderBindingLayout& rhs) const;

        // Returns true if this layout has at least one binding slot.
        bool HasBindings() const;

    private:

        struct ResourceBinding
        {
            std::string     name;
            std::uint32_t   slot;
        };

    private:

        std::uint8_t                    numUniformBindings_         = 0;
        std::uint8_t                    numUniformBlockBindings_    = 0;
        std::uint8_t                    numShaderStorageBindings_   = 0;
        std::vector<ResourceBinding>    bindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
