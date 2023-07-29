/*
 * GLShaderPipeline.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

void GLShaderPipeline::BuildSignature(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation)
{
    signature_.Build(numShaders, shaders, permutation);
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
