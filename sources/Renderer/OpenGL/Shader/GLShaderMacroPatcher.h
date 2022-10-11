/*
 * GLShaderMacroPatcher.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_MACRO_PATCHER_H
#define LLGL_GL_SHADER_MACRO_PATCHER_H


#include <string>


namespace LLGL
{


struct ShaderMacro;

// Allow to insert macro definitions after the "#version'-directive in GLSL source.
class GLShaderMacroPatcher
{

    public:

        // Initialies the patcher with the specified shader source.
        GLShaderMacroPatcher(const char* source);

        // Adds the specifies macro definitions to the shader source.
        void AddDefines(const ShaderMacro* defines);

    public:

        // Returns the current shader source as null terminated string.
        inline const char* GetSource() const
        {
            return source_.c_str();
        }

    private:

        std::string source_;
        std::size_t posAfterVersion_ = std::string::npos;

};


} // /namespace LLGL


#endif



// ================================================================================
