/*
 * GLShaderBindingLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_BINDING_LAYOUT_H
#define LLGL_GL_SHADER_BINDING_LAYOUT_H


#include <LLGL/Container/String.h>
#include <LLGL/Container/Vector.h>
#include <cstdint>
#include <memory>
#include "../RenderState/GLPipelineLayout.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderBindingLayout;
class GLStateManager;
class GLShaderBufferInterfaceMap;

using GLShaderBindingLayoutSPtr = std::shared_ptr<GLShaderBindingLayout>;

// Helper class to handle uniform block bindings and other resource bindings for GL shader programs with different pipeline layouts.
class GLShaderBindingLayout
{

    public:

        GLShaderBindingLayout() = default;
        GLShaderBindingLayout(const GLShaderBindingLayout&) = default;
        GLShaderBindingLayout& operator = (const GLShaderBindingLayout&) = default;

        GLShaderBindingLayout(const GLPipelineLayout& pipelineLayout);

        // Binds the resource slots to the specified GL shader program. Provides optional state manager if specified program is not currently bound, i.e. glUseProgram.
        void UniformAndBlockBinding(GLuint program, const GLShaderBufferInterfaceMap* bufferInterfaceMap = nullptr, GLStateManager* stateMngr = nullptr) const;

        // Returns true if this layout has at least one binding slot.
        bool HasBindings() const;

        // Returns true if this layout has at least one shader storage binding slot.
        bool HasShaderStorageBindings() const;

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const GLShaderBindingLayout& lhs, const GLShaderBindingLayout& rhs);

    private:

        struct NamedResourceBinding
        {
            string          name;
            std::uint32_t   slot;
            std::uint32_t   size; // Number of array elements
        };

    private:

        void BuildUniformBindings(const GLPipelineLayout& pipelineLayout);
        void BuildUniformBlockBindings(const GLPipelineLayout& pipelineLayout);
        void BuildShaderStorageBindings(const GLPipelineLayout& pipelineLayout);

        void AppendUniformBinding(const string& name, std::uint32_t slot, std::uint32_t size = 1u);
        void AppendUniformBlockBinding(const string& name, std::uint32_t slot);
        void AppendShaderStorageBinding(const string& name, std::uint32_t slot);

    private:

        #if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS
        static void GLSetProgramUniformBinding(GLuint program, const NamedResourceBinding& resource);
        #endif

        static void GLSetUniformBinding(GLuint program, const NamedResourceBinding& resource);

    private:

        std::uint8_t                        numUniformBindings_         = 0;
        std::uint8_t                        numUniformBlockBindings_    = 0;
        std::uint8_t                        numShaderStorageBindings_   = 0;
        vector<NamedResourceBinding>   bindings_;

};


} // /namespace LLGL


#endif



// ================================================================================
