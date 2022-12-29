/*
 * GLPipelineSignature.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLPipelineSignature.h"
#include "GLShader.h"
#include "../../../Core/HelperMacros.h"
#include <LLGL/Misc/ForRange.h>
#include <LLGL/Misc/TypeNames.h>
#include <stdexcept>


namespace LLGL
{


static GLuint SortShaderArray(std::size_t numShaders, const Shader* const* shaders, GLuint* outShaderIDs)
{
    constexpr auto firstShaderType   = static_cast<int>(ShaderType::Vertex);
    constexpr auto lastShaderType    = static_cast<int>(ShaderType::Compute);
    constexpr auto numShaderTypes    = (lastShaderType - firstShaderType + 1);

    /* Put all shaders into order */
    const GLShader* shadersOrderedByType[numShaderTypes] = {};
    for_range(i, numShaders)
    {
        const auto shaderType   = shaders[i]->GetType();
        const auto shaderIndex  = static_cast<int>(shaderType);
        if (shadersOrderedByType[shaderIndex] != nullptr)
            throw std::invalid_argument("duplicate definitions of " + std::string(ToString(shaderType)) + " shader in one pipeline");
        shadersOrderedByType[shaderIndex] = LLGL_CAST(const GLShader*, shaders[i]);
    }

    /* Condense output by omitting unused shader stages */
    GLuint outIndex = 0;

    for_range(i, numShaderTypes)
    {
        if (shadersOrderedByType[i] != nullptr)
        {
            if (outIndex == LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE)
                throw std::invalid_argument("exceeded maximum number of shaders in one pipeline");
            outShaderIDs[outIndex++] = shadersOrderedByType[i]->GetID();
        }
    }

    return outIndex;
}

GLPipelineSignature::GLPipelineSignature(std::size_t numShaders, const Shader* const* shaders)
{
    Build(numShaders, shaders);
}

void GLPipelineSignature::Build(std::size_t numShaders, const Shader* const* shaders)
{
    if (numShaders > LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE)
        throw std::out_of_range("numShaders must be less than or equal to LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE");
    numShaders_ = SortShaderArray(numShaders, shaders, shaders_);
}

int GLPipelineSignature::CompareSWO(const GLPipelineSignature& lhs, const GLPipelineSignature& rhs)
{
    LLGL_COMPARE_BOOL_MEMBER_SWO(numShaders_);
    for_range(i, lhs.numShaders_)
    {
        LLGL_COMPARE_BOOL_MEMBER_SWO(shaders_[i]);
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
