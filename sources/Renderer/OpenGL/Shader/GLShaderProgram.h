/*
 * GLShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_PROGRAM_H
#define LLGL_GL_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "GLShaderUniform.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderBindingLayout;

class GLShaderProgram final : public ShaderProgram
{

    public:

        void SetName(const char* name) override;

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;
        UniformLocation QueryUniformLocation(const char* name) const override;

        bool SetWorkGroupSize(const Extent3D& workGroupSize) override;
        bool GetWorkGroupSize(Extent3D& workGroupSize) const override;

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
        void BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats);
        void Link();

        bool QueryActiveAttribs(
            GLenum              attribCountType,
            GLenum              attribNameLengthType,
            GLint&              numAttribs,
            GLint&              maxNameLength,
            std::vector<char>&  nameBuffer
        ) const;

        void BuildTransformFeedbackVaryingsEXT(const std::vector<StreamOutputAttribute>& attributes);
        #ifndef __APPLE__
        void BuildTransformFeedbackVaryingsNV(const std::vector<StreamOutputAttribute>& attributes);
        #endif

        void Reflect(ShaderReflectionDescriptor& reflection) const;
        void QueryVertexAttributes(ShaderReflectionDescriptor& reflection) const;
        void QueryStreamOutputAttributes(ShaderReflectionDescriptor& reflection) const;
        void QueryConstantBuffers(ShaderReflectionDescriptor& reflection) const;
        void QueryStorageBuffers(ShaderReflectionDescriptor& reflection) const;
        void QueryUniforms(ShaderReflectionDescriptor& reflection) const;

        #ifdef GL_ARB_program_interface_query
        void QueryBufferProperties(ShaderReflectionDescriptor::ResourceView& resourceView, GLenum programInterface, GLuint resourceIndex) const;
        #endif // /GL_ARB_program_interface_query

    private:

        GLuint              id_                 = 0;
        StreamOutputFormat  streamOutputFormat_;

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
