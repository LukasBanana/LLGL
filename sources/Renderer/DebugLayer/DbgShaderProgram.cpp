/*
 * DbgShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShaderProgram.h"
#include "DbgShader.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


DbgShaderProgram::DbgShaderProgram(ShaderProgram& instance, RenderingDebugger* debugger) :
    instance    ( instance ),
    debugger_   ( debugger )
{
}

void DbgShaderProgram::AttachShader(Shader& shader)
{
    auto& shaderDbg = LLGL_CAST(DbgShader&, shader);
    DebugShaderAttachment(shaderDbg, __FUNCTION__);
    instance.AttachShader(shaderDbg.instance);
}

bool DbgShaderProgram::LinkShaders()
{
    DebugShaderComposition(__FUNCTION__);
    linked_ = instance.LinkShaders();
    return linked_;
}

std::string DbgShaderProgram::QueryInfoLog()
{
    return instance.QueryInfoLog();
}

std::vector<VertexAttribute> DbgShaderProgram::QueryVertexAttributes() const
{
    return instance.QueryVertexAttributes();
}

std::vector<ConstantBufferViewDescriptor> DbgShaderProgram::QueryConstantBuffers() const
{
    return instance.QueryConstantBuffers();
}

std::vector<StorageBufferViewDescriptor> DbgShaderProgram::QueryStorageBuffers() const
{
    return instance.QueryStorageBuffers();
}

std::vector<UniformDescriptor> DbgShaderProgram::QueryUniforms() const
{
    return instance.QueryUniforms();
}

void DbgShaderProgram::BuildInputLayout(const std::vector<VertexAttribute>& vertexAttribs)
{
    vertexLayout_.attributes    = vertexAttribs;
    vertexLayout_.bound         = true;

    instance.BuildInputLayout(vertexAttribs);
}

void DbgShaderProgram::BindConstantBuffer(const std::string& name, unsigned int bindingIndex)
{
    instance.BindConstantBuffer(name, bindingIndex);
}

void DbgShaderProgram::BindStorageBuffer(const std::string& name, unsigned int bindingIndex)
{
    instance.BindStorageBuffer(name, bindingIndex);
}

ShaderUniform* DbgShaderProgram::LockShaderUniform()
{
    auto shaderUniform = instance.LockShaderUniform();
    if (!shaderUniform)
        LLGL_DBG_WARN_HERE(WarningType::PointlessOperation, "renderer does not support individual shader uniforms");
    return shaderUniform;
}

void DbgShaderProgram::UnlockShaderUniform()
{
    return instance.UnlockShaderUniform();
}


/*
 * ======= Private: =======
 */

#define LLGL_SHADERTYPE_MASK(TYPE)  (1 << static_cast<int>(TYPE))
#define LLGL_VS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Vertex)
#define LLGL_PS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Fragment)
#define LLGL_HS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::TessControl)
#define LLGL_DS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::TessEvaluation)
#define LLGL_GS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Geometry)
#define LLGL_CS_MASK                LLGL_SHADERTYPE_MASK(ShaderType::Compute)

void DbgShaderProgram::DebugShaderAttachment(DbgShader& shaderDbg, const std::string& source)
{
    /* Check compilation state */
    if (!shaderDbg.IsCompiled())
        LLGL_DBG_ERROR(ErrorType::InvalidState, "attempt to attach uncompiled shader to shader program", source);

    /* Check if shader type already has been attached */
    for (auto other : shaderTypes_)
    {
        if (other == shaderDbg.GetType())
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "duplicate shader type attachments in shader program", source);
    }

    /* Add shader type to list */
    shaderTypes_.push_back(shaderDbg.GetType());

    /* Update shader attachment mask */
    shaderAttachmentMask_ |= LLGL_SHADERTYPE_MASK(shaderDbg.GetType());
}

void DbgShaderProgram::DebugShaderComposition(const std::string& source)
{
    /* Validate shader composition by shader attachment bit mask */
    switch (shaderAttachmentMask_)
    {
        case (LLGL_VS_MASK | LLGL_PS_MASK):
        case (LLGL_VS_MASK | LLGL_PS_MASK | LLGL_GS_MASK):
        case (LLGL_VS_MASK | LLGL_PS_MASK | LLGL_HS_MASK | LLGL_DS_MASK):
        case (LLGL_VS_MASK | LLGL_PS_MASK | LLGL_HS_MASK | LLGL_DS_MASK | LLGL_GS_MASK):
        case (LLGL_CS_MASK):
            break;
        default:
            LLGL_DBG_ERROR(ErrorType::InvalidState, "invalid shader composition", source);
            break;
    }
}

#undef LLGL_SHADERTYPE_MASK
#undef LLGL_VS_MASK
#undef LLGL_PS_MASK
#undef LLGL_HS_MASK
#undef LLGL_DS_MASK
#undef LLGL_GS_MASK
#undef LLGL_CS_MASK


} // /namespace LLGL



// ================================================================================
