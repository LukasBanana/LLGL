/*
 * GLShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_PROGRAM_H
#define LLGL_GL_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "GLShaderUniform.h"
#include "../OpenGL.h"


namespace LLGL
{


struct GLShaderAttribute;
class GLShaderBindingLayout;

class GLShaderProgram final : public ShaderProgram
{

    public:

        void SetName(const char* name) override;

        bool HasErrors() const override;
        std::string GetReport() const override;

        bool Reflect(ShaderReflection& reflection) const override;
        UniformLocation FindUniformLocation(const char* name) const override;

    public:

        GLShaderProgram(const ShaderProgramDescriptor& desc);
        ~GLShaderProgram();

        /*
        Updates all uniform/storage block bindings and resources by the specified binding layout.
        This shader program must already be bound with the GLStateManager.
        */
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout) const;

        // Returns the shader program ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        void Attach(Shader* shader);
        void BindAttribLocations(std::size_t numVertexAttribs, const GLShaderAttribute* vertexAttribs);
        void BindFragDataLocations(std::size_t numFragmentAttribs, const GLShaderAttribute* fragmentAttribs);
        void LinkProgram(std::size_t numVaryings, const char* const* varyings);

        bool QueryActiveAttribs(
            GLenum              attribCountType,
            GLenum              attribNameLengthType,
            GLint&              numAttribs,
            GLint&              maxNameLength,
            std::vector<char>&  nameBuffer
        ) const;

        void BuildTransformFeedbackVaryingsEXT(std::size_t numVaryings, const char* const* varyings);
        #ifdef GL_NV_transform_feedback
        void BuildTransformFeedbackVaryingsNV(std::size_t numVaryings, const char* const* varyings);
        #endif

        void QueryReflection(ShaderReflection& reflection) const;
        void QueryVertexAttributes(ShaderReflection& reflection) const;
        void QueryStreamOutputAttributes(ShaderReflection& reflection) const;
        void QueryConstantBuffers(ShaderReflection& reflection) const;
        void QueryStorageBuffers(ShaderReflection& reflection) const;
        void QueryUniforms(ShaderReflection& reflection) const;
        void QueryWorkGroupSize(ShaderReflection& reflection) const;

        #ifdef GL_ARB_program_interface_query
        void QueryBufferProperties(ShaderResource& resource, GLenum programInterface, GLuint resourceIndex) const;
        #endif

    private:

        GLuint id_ = 0;

    private:

        /*
        Reference to active binding layout is mutable since it's only to track state changes
        TODO: try to avoid <mutable> keyword, maybe there's a better way for the purpose of this member
        */
        mutable const GLShaderBindingLayout* bindingLayout_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
