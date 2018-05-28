/*
 * DbgShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShaderProgram.h"
#include "DbgShader.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


DbgShaderProgram::DbgShaderProgram(ShaderProgram& instance, RenderingDebugger* debugger) :
    instance  { instance },
    debugger_ { debugger }
{
}

void DbgShaderProgram::AttachShader(Shader& shader)
{
    auto& shaderDbg = LLGL_CAST(DbgShader&, shader);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderAttachment(shaderDbg);
    }

    instance.AttachShader(shaderDbg.instance);
}

void DbgShaderProgram::DetachAll()
{
    instance.DetachAll();

    /* Reset debug information */
    shaderAttachmentMask_   = 0;
    linked_                 = false;
    shaderTypes_.clear();
    vertexLayout_.attributes.clear();
    vertexLayout_.bound = false;
}

bool DbgShaderProgram::LinkShaders()
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderComposition();
    }

    linked_ = instance.LinkShaders();

    return linked_;
}

std::string DbgShaderProgram::QueryInfoLog()
{
    return instance.QueryInfoLog();
}

ShaderReflectionDescriptor DbgShaderProgram::QueryReflectionDesc() const
{
    return instance.QueryReflectionDesc();
}

#if 1//TODO: remove
std::vector<VertexAttribute> DbgShaderProgram::QueryVertexAttributes() const
{
    return instance.QueryVertexAttributes();
}

std::vector<StreamOutputAttribute> DbgShaderProgram::QueryStreamOutputAttributes() const
{
    return instance.QueryStreamOutputAttributes();
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
#endif

void DbgShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats)
{
    for (std::uint32_t i = 0; i < numVertexFormats; ++i)
    {
        for (const auto& attrib : vertexFormats[i].attributes)
            vertexLayout_.attributes.push_back(attrib);
    }

    vertexLayout_.bound = true;

    instance.BuildInputLayout(numVertexFormats, vertexFormats);
}

void DbgShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    instance.BindConstantBuffer(name, bindingIndex);
}

void DbgShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    instance.BindStorageBuffer(name, bindingIndex);
}

ShaderUniform* DbgShaderProgram::LockShaderUniform()
{
    return instance.LockShaderUniform();
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

void DbgShaderProgram::DebugShaderAttachment(DbgShader& shaderDbg)
{
    /* Check compilation state */
    if (!shaderDbg.IsCompiled())
        LLGL_DBG_ERROR(ErrorType::InvalidState, "attempt to attach uncompiled shader to shader program");

    /* Check if shader type already has been attached */
    for (auto other : shaderTypes_)
    {
        if (other == shaderDbg.GetType())
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "duplicate shader type attachments in shader program");
    }

    /* Add shader type to list */
    shaderTypes_.push_back(shaderDbg.GetType());

    /* Update shader attachment mask */
    shaderAttachmentMask_ |= LLGL_SHADERTYPE_MASK(shaderDbg.GetType());
}

void DbgShaderProgram::DebugShaderComposition()
{
    /* Validate shader composition by shader attachment bit mask */
    switch (shaderAttachmentMask_)
    {
        case ( LLGL_VS_MASK                                                             ):
        case ( LLGL_VS_MASK |                               LLGL_GS_MASK                ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK                               ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK | LLGL_GS_MASK                ):
        case ( LLGL_VS_MASK |                                              LLGL_PS_MASK ):
        case ( LLGL_VS_MASK |                               LLGL_GS_MASK | LLGL_PS_MASK ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK |                LLGL_PS_MASK ):
        case ( LLGL_VS_MASK | LLGL_HS_MASK | LLGL_DS_MASK | LLGL_GS_MASK | LLGL_PS_MASK ):
        case ( LLGL_CS_MASK ):
            break;
        default:
            LLGL_DBG_ERROR(ErrorType::InvalidState, "invalid shader composition");
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
