/*
 * GLPipelineSignature.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLPipelineSignature.h"
#include "GLShader.h"
#include "../../CheckedCast.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/TypeNames.h>
#include <cstring>


namespace LLGL
{


static GLuint SortShaderArray(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation, GLuint* outShaderIDs)
{
    constexpr auto numShaderTypes = (static_cast<int>(ShaderType::Compute) + 1);

    /* Find shaders that are affected by permutation */
    const GLShader* finalGLPositionShader = nullptr;
    if (permutation == GLShader::PermutationFlippedYPosition)
        finalGLPositionShader = GLPipelineSignature::FindFinalGLPositionShader(numShaders, shaders);

    /* Put all shaders into order */
    const GLShader* shadersOrderedByType[numShaderTypes] = {};
    for_range(i, numShaders)
    {
        LLGL_ASSERT_PTR(shaders[i]);
        const ShaderType shaderType = shaders[i]->GetType();
        const int shaderIndex = static_cast<int>(shaderType);
        LLGL_ASSERT(shadersOrderedByType[shaderIndex] == nullptr, "duplicate definitions of %s shader in one pipeline", ToString(shaderType));
        shadersOrderedByType[shaderIndex] = LLGL_CAST(const GLShader*, shaders[i]);
    }

    /* Condense output by omitting unused shader stages */
    GLuint outIndex = 0;

    for_range(i, numShaderTypes)
    {
        if (shadersOrderedByType[i] != nullptr)
        {
            LLGL_ASSERT(outIndex != LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE, "exceeded maximum number of shaders in one pipeline");
            const GLShader::Permutation permutationForShader =
            (
                shadersOrderedByType[i] == finalGLPositionShader
                    ? GLShader::PermutationFlippedYPosition
                    : GLShader::PermutationDefault
            );
            outShaderIDs[outIndex++] = shadersOrderedByType[i]->GetID(permutationForShader);
        }
    }

    return outIndex;
}

GLPipelineSignature::GLPipelineSignature(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation, void* /*pipelineCache*/)
{
    Build(numShaders, shaders, permutation);
}

// Returns true if the specified array of shaders contains a separable shader
static bool HasSeparableShaders(std::size_t numShaders, const Shader* const* shaders)
{
    return (numShaders > 0 && LLGL_CAST(const GLShader*, shaders[0])->IsSeparable());
}

void GLPipelineSignature::Build(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation)
{
    LLGL_ASSERT(numShaders <= LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE);
    data_.isSeparablePipeline = HasSeparableShaders(numShaders, shaders);
    data_.numShaders = SortShaderArray(numShaders, shaders, permutation, data_.shaders);
}

int GLPipelineSignature::CompareSWO(const GLPipelineSignature& lhs, const GLPipelineSignature& rhs)
{
    return std::memcmp(&(lhs.data_), &(rhs.data_), sizeof(GLuint)*(1 + lhs.GetNumShaders()));
}

static int GetShaderPipelineOrder(const Shader* shader)
{
    /* Convert shader type to order number */
    static_assert(ShaderType::Vertex         < ShaderType::TessControl,    "ShaderType::Vertex must have lower value than ShaderType::TessControl"        );
    static_assert(ShaderType::TessControl    < ShaderType::TessEvaluation, "ShaderType::TessControl must have lower value than ShaderType::TessEvaluation");
    static_assert(ShaderType::TessEvaluation < ShaderType::Geometry,       "ShaderType::TessEvaluation must have lower value than ShaderType::Geometry"   );
    static_assert(ShaderType::Geometry       < ShaderType::Fragment,       "ShaderType::Geometry must have lower value than ShaderType::Fragment"         );
    return (shader != nullptr ? static_cast<int>(shader->GetType()) : 0);
}

const GLShader* GLPipelineSignature::FindFinalGLPositionShader(std::size_t numShaders, const Shader* const* shaders)
{
    const GLShader* finalGLPositionShader = nullptr;

    for_range(i, numShaders)
    {
        if (const Shader* shader = shaders[i])
        {
            const GLShader* shaderGL = LLGL_CAST(const GLShader*, shader);
            switch (shaderGL->GetType())
            {
                case ShaderType::Vertex:
                case ShaderType::TessEvaluation:
                case ShaderType::Geometry:
                    if (GetShaderPipelineOrder(finalGLPositionShader) < GetShaderPipelineOrder(shaderGL))
                        finalGLPositionShader = shaderGL;
                    break;
                default:
                    break;
            }
        }
    }

    return finalGLPositionShader;
}


} // /namespace LLGL



// ================================================================================
