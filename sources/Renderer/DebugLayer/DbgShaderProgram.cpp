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


DbgShaderProgram::DbgShaderProgram(ShaderProgram& instance, RenderingDebugger* debugger, const GraphicsShaderProgramDescriptor& desc) :
    instance  { instance },
    debugger_ { debugger }
{
    /* Debug all attachments and shader composition */
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderAttachment(desc.vertexShader);
        DebugShaderAttachment(desc.tessControlShader);
        DebugShaderAttachment(desc.tessEvaluationShader);
        DebugShaderAttachment(desc.geometryShader);
        DebugShaderAttachment(desc.fragmentShader);
        DebugShaderComposition();
    }

    /* Store all attributes of vertex layout */
    for (const auto& format : desc.vertexFormats)
    {
        for (const auto& attrib : format.attributes)
            vertexLayout_.attributes.push_back(attrib);
    }
    vertexLayout_.bound = true;
}

DbgShaderProgram::DbgShaderProgram(ShaderProgram& instance, RenderingDebugger* debugger, const ComputeShaderProgramDescriptor& desc) :
    instance  { instance },
    debugger_ { debugger }
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderAttachment(desc.computeShader);
        DebugShaderComposition();
    }
}

bool DbgShaderProgram::HasErrors() const
{
    return instance.HasErrors();
}

std::string DbgShaderProgram::QueryInfoLog()
{
    return instance.QueryInfoLog();
}

ShaderReflectionDescriptor DbgShaderProgram::QueryReflectionDesc() const
{
    return instance.QueryReflectionDesc();
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

void DbgShaderProgram::DebugShaderAttachment(Shader* shader)
{
    if (shader != nullptr)
    {
        auto& shaderDbg = LLGL_CAST(DbgShader&, *shader);

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
