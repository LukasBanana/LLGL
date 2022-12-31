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
#include "../../../Core/BasicReport.h"
#include "../../../Core/LinearStringContainer.h"
#include <functional>


namespace LLGL
{


struct GLShaderAttribute
{
    GLuint          index;
    const GLchar*   name;
};

// Callback interface for shader source patching.
using ShaderSourceCallback = std::function<void(const char* source)>;

class GLShader : public Shader
{

    public:

        const Report* GetReport() const override;

    public:

        // Returns the vertex input attributes:
        const GLShaderAttribute* GetVertexAttribs() const;
        std::size_t GetNumVertexAttribs() const;

        // Returns the vertex input attributes:
        const GLShaderAttribute* GetFragmentAttribs() const;
        std::size_t GetNumFragmentAttribs() const;

        // Returns the transform feedback varying names.
        inline const std::vector<const char*>& GetTransformFeedbackVaryings() const
        {
            return transformFeedbackVaryings_;
        }

        // Returns the native shader ID. Can be either from glCreateShader or glCreateShaderProgramv.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns true if this is a separable shader, i.e. of type <GLSeparableShader>. Otherwise, it's of type <GLLegacyShader>.
        inline bool IsSeparable() const
        {
            return isSeparable_;
        }

    public:

        // Patches the shader source and invokes the callback with the preprocessed shader.
        static void PatchShaderSource(
            const ShaderSourceCallback& sourceCallback,
            const ShaderDescriptor&     shaderDesc
        );

        // Patches the shader source with the specified options: macro definitions, pragma directives, additional statements etc.
        static void PatchShaderSourceWithOptions(
            const ShaderSourceCallback& sourceCallback,
            const char*                 source,
            const ShaderMacro*          defines,
            bool                        pragmaOptimizeOff   = false,
            const char*                 vertexTransformStmt = nullptr
        );

    protected:

        GLShader(const bool isSeparable, const ShaderDescriptor& desc);

        // Resets the report with the specified compile/link status and log.
        void ReportStatusAndLog(bool status, const std::string& log);

        // Stores the native shader ID.
        inline void SetID(GLuint id)
        {
            id_ = id;
        }

    private:

        void ReserveAttribs(const ShaderDescriptor& desc);
        void BuildVertexInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs);
        void BuildFragmentOutputLayout(std::size_t numFragmentAttribs, const FragmentAttribute* fragmentAttribs);
        void BuildTransformFeedbackVaryings(std::size_t numVaryings, const VertexAttribute* varyings);

    private:

        const bool                      isSeparable_;
        GLuint                          id_                         = 0; // ID from either glCreateShader or glCreateShaderProgramv
        LinearStringContainer           shaderAttribNames_;
        std::vector<GLShaderAttribute>  shaderAttribs_;
        std::size_t                     numVertexAttribs_           = 0;
        std::vector<const char*>        transformFeedbackVaryings_;
        BasicReport                     report_;

};


} // /namespace LLGL


#endif



// ================================================================================
