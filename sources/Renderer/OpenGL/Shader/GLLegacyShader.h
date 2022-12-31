/*
 * GLLegacyShader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_LEGACY_SHADER_PROGRAM_H
#define LLGL_GL_LEGACY_SHADER_PROGRAM_H


#include "GLShader.h"


namespace LLGL
{


// Shader implementation for legacy GL shaders (fallback when GL_ARB_separate_shader_objects is not available).
class GLLegacyShader final : public GLShader
{

    public:

        void SetName(const char* name) override;
        bool Reflect(ShaderReflection& reflection) const override;

    public:

        GLLegacyShader(const ShaderDescriptor& desc);
        ~GLLegacyShader();

    public:

        // Compiles a native GL shader from source.
        static void CompileShaderSource(GLuint shader, const char* source);

        // Returns true if the native GL shader was compiled successfully.
        static bool GetCompileStatus(GLuint shader);

        // Returns the native GL shader log.
        static std::string GetGLShaderLog(GLuint shader);

    private:

        void BuildShader(const ShaderDescriptor& shaderDesc);
        void CompileSource(const ShaderDescriptor& shaderDesc);
        void LoadBinary(const ShaderDescriptor& shaderDesc);

};


} // /namespace LLGL


#endif



// ================================================================================
