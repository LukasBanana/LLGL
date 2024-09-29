/*
 * DbgShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgShader.h"
#include "../DbgCore.h"


namespace LLGL
{


DbgShader::DbgShader(Shader& instance, const ShaderDescriptor& desc) :
    Shader    { desc.type            },
    instance  { instance             },
    desc      { desc                 },
    label     { LLGL_DBG_LABEL(desc) }
{
    switch (GetType())
    {
        case ShaderType::Vertex:
        case ShaderType::Fragment:
            CacheShaderReflection();
            break;
        default:
            break;
    }
}

void DbgShader::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}

const Report* DbgShader::GetReport() const
{
    return instance.GetReport();
}

bool DbgShader::Reflect(ShaderReflection& reflection) const
{
    return instance.Reflect(reflection);
}

const char* DbgShader::GetVertexID() const
{
    return (vertexID_.empty() ? nullptr : vertexID_.c_str());
}

const char* DbgShader::GetInstanceID() const
{
    return (instanceID_.empty() ? nullptr : instanceID_.c_str());
}

bool DbgShader::IsCompiled() const
{
    if (const Report* report = instance.GetReport())
        return !report->HasErrors();
    else
        return true;
}


/*
 * ======= Private: =======
 */

void DbgShader::CacheShaderReflection()
{
    ShaderReflection reflect;
    if (instance.Reflect(reflect))
    {
        for (const VertexAttribute& attr : reflect.vertex.inputAttribs)
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

        hasAnyOutputAttribs_ = !(reflect.vertex.outputAttribs.empty() && reflect.fragment.outputAttribs.empty());
    }
}


} // /namespace LLGL



// ================================================================================
