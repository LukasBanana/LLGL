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


DbgShaderProgram::DbgShaderProgram(
    ShaderProgram&                  instance,
    RenderingDebugger*              debugger,
    const ShaderProgramDescriptor&  desc,
    const RenderingCapabilities&    caps)
:   instance  { instance },
    debugger_ { debugger }
{
    /* Debug all attachments and shader composition */
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateShaderAttachment(desc.vertexShader);
        ValidateShaderAttachment(desc.tessControlShader);
        ValidateShaderAttachment(desc.tessEvaluationShader);
        ValidateShaderAttachment(desc.geometryShader);
        ValidateShaderAttachment(desc.fragmentShader);
        ValidateShaderAttachment(desc.computeShader);
        ValidateShaderComposition();
        QueryInstanceAndVertexIDs(caps);
    }

    /* Store all attributes of vertex layout */
    for (const auto& format : desc.vertexFormats)
    {
        for (const auto& attrib : format.attributes)
            vertexLayout_.attributes.push_back(attrib);
    }
    vertexLayout_.bound = true;
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

void DbgShaderProgram::ValidateShaderAttachment(Shader* shader)
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

void DbgShaderProgram::ValidateShaderComposition()
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

void DbgShaderProgram::QueryInstanceAndVertexIDs(const RenderingCapabilities& caps)
{
    auto HasShadingLang = [&caps](const ShadingLanguage lang)
    {
        return (std::find(caps.shadingLanguages.begin(), caps.shadingLanguages.end(), lang) != caps.shadingLanguages.end());
    };

    /* Store meta information if the instance ID or vertex ID is used in the shader program */
    if (HasShadingLang(ShadingLanguage::HLSL))
        QueryInstanceAndVertexIDs("SV_VertexID", "SV_InstanceID");
    if (HasShadingLang(ShadingLanguage::GLSL) || HasShadingLang(ShadingLanguage::ESSL))
        QueryInstanceAndVertexIDs("gl_VertexID", "gl_InstanceID");
    if (HasShadingLang(ShadingLanguage::SPIRV))
        QueryInstanceAndVertexIDs("gl_VertexIndex", "gl_InstanceIndex");
}

void DbgShaderProgram::QueryInstanceAndVertexIDs(const char* vertexIDName, const char* instanceIDName)
{
    try
    {
        auto reflect = instance.QueryReflectionDesc();

        for (const auto& attr : reflect.vertexAttributes)
        {
            if (vertexID_ == nullptr)
            {
                if (attr.name == vertexIDName)
                    vertexID_ = vertexIDName;
            }
            if (instanceID_ == nullptr)
            {
                if (attr.name == instanceIDName)
                    instanceID_ = instanceIDName;
            }
            if (vertexID_ != nullptr && instanceID_ != nullptr)
                break;
        }
    }
    catch (const std::runtime_error&)
    {
        // ignore here
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
