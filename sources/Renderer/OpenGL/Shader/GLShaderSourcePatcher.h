/*
 * GLShaderSourcePatcher.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_SOURCE_PATCHER_H
#define LLGL_GL_SHADER_SOURCE_PATCHER_H


#include <string>


namespace LLGL
{


struct ShaderMacro;

// Allows to insert source code into a given GLSL shader.
class GLShaderSourcePatcher
{

    public:

        // Initialies the patcher with the specified shader source.
        GLShaderSourcePatcher(const char* source);

        // Overrides the version directive (or adds it if it's missing), e.g "300 es" turns into "#version 300 es".
        void OverrideVersion(const char* version);

        // Adds the specifies macro definitions to the shader source.
        void AddDefines(const ShaderMacro* defines);

        // Adds a '#pragma' directive after the '#version' directive and after the last added definitions.
        void AddPragmaDirective(const char* statement);

        // Adds the specified statement to all source positions that determine a vertex position, e.g. "gl_Position.y = -gl_Position.y;".
        // NOTE: This patcher cannot handle shader source whose entry point or its return statements are modified by preprocessor directives,
        //       i.e. no preprocessing is performed prior to scanning the source.
        void AddFinalVertexTransformStatements(const char* statement);

    public:

        // Returns the current shader source as null terminated string.
        inline const char* GetSource() const
        {
            return source_.c_str();
        }

    private:

        // Inserts the specified statement after the '#version'-directive and any previously inserted statement.
        void InsertAfterVersionDirective(const std::string& statement);

        // Finds and stores the source location of the entry point, i.e. points to the first character after of the entry point declaration "void main()".
        void CacheEntryPointSourceLocation();

        // Updates the source insertion points.
        void ResetInsertionPoints(std::size_t newStatementInsertPos);

    private:

        std::string source_;
        std::size_t statementInsertPos_ = 0;
        std::size_t entryPointStartPos_ = std::string::npos;

};


} // /namespace LLGL


#endif



// ================================================================================
