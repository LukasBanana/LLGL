/*
 * DbgShader.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShader.h"
#include "../DbgCore.h"


namespace LLGL
{


DbgShader::DbgShader(Shader& instance, const ShaderDescriptor& desc) :
    Shader    { desc.type },
    instance  { instance  },
    desc      { desc      }
{
    if (GetType() == ShaderType::Vertex)
        QueryInstanceAndVertexIDs();
}

void DbgShader::SetName(const char* name)
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

UniformLocation DbgShader::FindUniformLocation(const char* name) const
{
    return instance.FindUniformLocation(name);
}

bool DbgShader::IsPostTessellationVertex() const
{
    return instance.IsPostTessellationVertex();
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
    if (auto report = instance.GetReport())
        return !report->HasErrors();
    else
        return true;
}


/*
 * ======= Private: =======
 */

void DbgShader::QueryInstanceAndVertexIDs()
{
    ShaderReflection reflect;
    #if 0 //TODO
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
    #endif
}


} // /namespace LLGL



// ================================================================================
