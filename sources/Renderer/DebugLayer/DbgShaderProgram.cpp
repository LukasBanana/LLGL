/*
 * DbgShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShaderProgram.h"
#include "DbgShader.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include <LLGL/Strings.h>


namespace LLGL
{


DbgShaderProgram::DbgShaderProgram(
    ShaderProgram&                  instance,
    RenderingDebugger*              debugger,
    const ShaderProgramDescriptor&  desc)
:
    instance  { instance },
    debugger_ { debugger }
{
    /* Debug all attachments and shader composition */
    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        /* Validate all attached shaders */
        ValidateShaderAttachment(desc.vertexShader, ShaderType::Vertex);
        ValidateShaderAttachment(desc.tessControlShader, ShaderType::TessControl);
        ValidateShaderAttachment(desc.tessEvaluationShader, ShaderType::TessEvaluation);
        ValidateShaderAttachment(desc.geometryShader, ShaderType::Geometry);
        ValidateShaderAttachment(desc.fragmentShader, ShaderType::Fragment);
        ValidateShaderAttachment(desc.computeShader, ShaderType::Compute);

        /* Validate shader composition */
        Shader* shaders[] =
        {
            desc.vertexShader,
            desc.tessControlShader,
            desc.tessEvaluationShader,
            desc.geometryShader,
            desc.fragmentShader,
            desc.computeShader
        };

        if (!ShaderProgram::ValidateShaderComposition(shaders, sizeof(shaders)/sizeof(shaders[0])))
            LLGL_DBG_ERROR(ErrorType::InvalidState, "invalid shader composition");

        QueryInstanceAndVertexIDs();
    }

    /* Store all attributes of vertex layout */
    if (auto shader = desc.vertexShader)
    {
        auto shaderDbg = LLGL_CAST(DbgShader*, desc.vertexShader);
        vertexLayout_.attributes    = shaderDbg->desc.vertex.inputAttribs;
        vertexLayout_.bound         = true;
    }

    /* Store information whether this shader program contains a fragment shader */
    hasFragmentShader_ = (desc.fragmentShader != nullptr && desc.fragmentShader->GetType() == ShaderType::Fragment);
}

bool DbgShaderProgram::HasErrors() const
{
    return instance.HasErrors();
}

std::string DbgShaderProgram::GetReport() const
{
    return instance.GetReport();
}

bool DbgShaderProgram::Reflect(ShaderReflection& reflection) const
{
    return instance.Reflect(reflection);
}

UniformLocation DbgShaderProgram::FindUniformLocation(const char* name) const
{
    return instance.FindUniformLocation(name);
}

const char* DbgShaderProgram::GetVertexID() const
{
    return (vertexID_.empty() ? nullptr : vertexID_.c_str());
}

const char* DbgShaderProgram::GetInstanceID() const
{
    return (instanceID_.empty() ? nullptr : instanceID_.c_str());
}


/*
 * ======= Private: =======
 */

void DbgShaderProgram::ValidateShaderAttachment(Shader* shader, const ShaderType type)
{
    if (shader != nullptr)
    {
        auto& shaderDbg = LLGL_CAST(DbgShader&, *shader);

        /* Check compilation state */
        if (!shaderDbg.IsCompiled())
            LLGL_DBG_ERROR(ErrorType::InvalidState, "attempt to attach uncompiled shader to shader program");

        /* Check if shader type already has been attached */
        if (shaderDbg.GetType() != type)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "mismatch between shader type (" + std::string(ToString(shaderDbg.GetType())) +
                ") and shader program attachment (" + std::string(ToString(type)) + ")"
            );
        }
    }
}

void DbgShaderProgram::QueryInstanceAndVertexIDs()
{
    ShaderReflection reflect;
    if (instance.Reflect(reflect))
    {
        for (const auto& attr : reflect.vertex.inputAttribs)
        {
            if (vertexID_.empty())
            {
                if (attr.systemValue == SystemValue::VertexID)
                    vertexID_ = attr.name;
            }
            if (instanceID_.empty())
            {
                if (attr.systemValue == SystemValue::InstanceID)
                    instanceID_ = attr.name;
            }
            if (!vertexID_.empty() && !instanceID_.empty())
                break;
        }
    }
}


} // /namespace LLGL



// ================================================================================
