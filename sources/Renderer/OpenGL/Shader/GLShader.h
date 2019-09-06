/*
 * GLShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_H
#define LLGL_GL_SHADER_H


#include <LLGL/Shader.h>
#include "../OpenGL.h"
#include "../../../Core/LinearStringContainer.h"


namespace LLGL
{


struct GLVertexAttribute
{
    GLuint          index;
    const GLchar*   name;
};

class GLShader final : public Shader
{

    public:

        void SetName(const char* name) override;

        bool HasErrors() const override;

        std::string GetReport() const override;

    public:

        GLShader(const ShaderDescriptor& desc);
        ~GLShader();

        // Returns the native shader ID.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns the vertex input attributes.
        inline const std::vector<GLVertexAttribute>& GetVertexAttribs() const
        {
            return vertexAttribs_;
        }

        // Returns the transform feedback varying names.
        inline const std::vector<const char*>& GetTransformFeedbackVaryings() const
        {
            return transformFeedbackVaryings_;
        }

    private:

        void BuildShader(const ShaderDescriptor& shaderDesc);
        void ReserveAttribNames(const VertexShaderAttributes& vertexAttribs);
        void BuildInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs);
        void BuildTransformFeedbackVaryings(std::size_t numVaryings, const VertexAttribute* varyings);

        void CompileSource(const ShaderDescriptor& shaderDesc);
        void LoadBinary(const ShaderDescriptor& shaderDesc);

    private:

        GLuint                          id_                         = 0;

        LinearStringContainer           vertexAttribNames_;
        std::vector<GLVertexAttribute>  vertexAttribs_;
        std::vector<const char*>        transformFeedbackVaryings_;

};


} // /namespace LLGL


#endif



// ================================================================================
