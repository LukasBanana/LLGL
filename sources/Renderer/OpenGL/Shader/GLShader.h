/*
 * GLShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_H
#define LLGL_GL_SHADER_H


#include <LLGL/Shader.h>
#include <LLGL/Report.h>
#include "../OpenGL.h"
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

        // Enumeration of all GL shader permutations.
        enum Permutation
        {
            PermutationDefault = 0,         // Default GL shader; Unmodified source.
            PermutationFlippedYPosition,    // Shader permutation with flipped Y position; Implemented as 'gl_Position.y = -gl_Position.y' statements.
            PermutationCount,
        };

    public:

        const Report* GetReport() const override;

    public:

        // Returns the vertex input attributes:
        const GLShaderAttribute* GetVertexAttribs() const;
        std::size_t GetNumVertexAttribs() const;

        // Returns the vertex input attributes:
        const GLShaderAttribute* GetFragmentAttribs() const;
        std::size_t GetNumFragmentAttribs() const;

        // Returns the GLenum for this shader type, e.g. GL_VERTEX_SHADER.
        GLenum GetGLType() const;

        // Returns the transform feedback varying names.
        inline const std::vector<const char*>& GetTransformFeedbackVaryings() const
        {
            return transformFeedbackVaryings_;
        }

        // Returns the native shader ID. Can be either from glCreateShader or glCreateShaderProgramv.
        inline GLuint GetID() const
        {
            return id_[PermutationDefault];
        }

        // Returns the native shader ID for the specified permutation or the default permutation if the specified one is not available.
        inline GLuint GetID(Permutation permutation) const
        {
            return (id_[permutation] != 0 ? id_[permutation] : id_[PermutationDefault]);
        }

        // Returns true if this is a separable shader, i.e. of type <GLSeparableShader>. Otherwise, it's of type <GLLegacyShader>.
        inline bool IsSeparable() const
        {
            return isSeparable_;
        }

    public:

        // Returns true if the specified shader descriptor requires the permutation with flipped Y-position; See PermutationFlippedYPosition.
        static bool NeedsPermutationFlippedYPosition(const ShaderType shaderType, long shaderFlags);

        // Returns true if any of the specified shaders has the specified permutation.
        static bool HasAnyShaderPermutation(Permutation permutation, const ArrayView<Shader*>& shaders);

        // Patches the shader source and invokes the callback with the preprocessed shader. See ShaderCompileFlags.
        static void PatchShaderSource(
            const ShaderSourceCallback& sourceCallback,
            const char*                 shaderSource,
            const ShaderDescriptor&     shaderDesc,
            long                        enabledFlags
        );

        // Patches the shader source with the specified options: macro definitions, pragma directives, additional statements etc.
        static void PatchShaderSourceWithOptions(
            const ShaderSourceCallback& sourceCallback,
            const char*                 source,
            const ShaderMacro*          defines,
            bool                        pragmaOptimizeOff   = false,
            const char*                 vertexTransformStmt = nullptr,
            const char*                 versionOverride     = nullptr
        );

    protected:

        GLShader(const bool isSeparable, const ShaderDescriptor& desc);

        // Resets the report with the specified compile/link status and log.
        void ReportStatusAndLog(bool status, const std::string& log);

        // Stores the native shader ID.
        inline void SetID(GLuint id, Permutation permutation = PermutationDefault)
        {
            id_[permutation] = id;
        }

    private:

        void ReserveAttribs(const ShaderDescriptor& desc);
        bool BuildVertexInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs);
        void BuildFragmentOutputLayout(std::size_t numFragmentAttribs, const FragmentAttribute* fragmentAttribs);
        void BuildTransformFeedbackVaryings(std::size_t numVaryings, const VertexAttribute* varyings);

    private:

        const bool                      isSeparable_;
        GLuint                          id_[PermutationCount]       = {}; // ID from either glCreateShader or glCreateShaderProgramv
        LinearStringContainer           shaderAttribNames_;
        std::vector<GLShaderAttribute>  shaderAttribs_;
        std::size_t                     numVertexAttribs_           = 0;
        std::vector<const char*>        transformFeedbackVaryings_;
        Report                          report_;

};


} // /namespace LLGL


#endif



// ================================================================================
