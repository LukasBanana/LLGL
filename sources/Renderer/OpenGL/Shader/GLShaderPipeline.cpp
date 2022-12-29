/*
 * GLShaderPipeline.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderPipeline.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLShaderPipeline::GLShaderPipeline(GLuint id) :
    id_ { id }
{
}

void GLShaderPipeline::BuildSignature(std::size_t numShaders, const Shader* const* shaders)
{
    signature_.Build(numShaders, shaders);
}

int GLShaderPipeline::CompareSWO(const GLShaderPipeline& lhs, const GLShaderPipeline& rhs)
{
    return GLPipelineSignature::CompareSWO(lhs.signature_, rhs.signature_);
}

int GLShaderPipeline::CompareSWO(const GLShaderPipeline& lhs, const GLPipelineSignature& rhs)
{
    return GLPipelineSignature::CompareSWO(lhs.signature_, rhs);
}


} // /namespace LLGL



// ================================================================================
