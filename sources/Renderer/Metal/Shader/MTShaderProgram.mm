/*
 * MTShaderProgram.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShaderProgram.h"
#include "MTShader.h"
#include "../../CheckedCast.h"


namespace LLGL
{


MTShaderProgram::MTShaderProgram()
{
}

MTShaderProgram::~MTShaderProgram()
{
    ReleaseVertexDesc();
}

void MTShaderProgram::AttachShader(Shader& shader)
{
    /* Get native shader function */
    auto& shaderMT = LLGL_CAST(MTShader&, shader);
    id<MTLFunction> shaderFunc = shaderMT.GetNative();
    
    /* Store reference to shader function */
    switch (shader.GetType())
    {
        case ShaderType::Vertex:
            vertexFunc_ = shaderFunc;
            break;
        case ShaderType::TessControl:
            //???
            break;
        case ShaderType::TessEvaluation:
            //???
            break;
        case ShaderType::Geometry:
            //???
            break;
        case ShaderType::Fragment:
            fragmentFunc_ = shaderFunc;
            break;
        case ShaderType::Compute:
            kernelFunc_ = shaderFunc;
            break;
    }
}

void MTShaderProgram::DetachAll()
{
    //todo
}

bool MTShaderProgram::LinkShaders()
{
    return true;//todo
}

std::string MTShaderProgram::QueryInfoLog()
{
    return "";//todo
}

//TODO
ShaderReflectionDescriptor MTShaderProgram::QueryReflectionDesc() const
{
    ShaderReflectionDescriptor reflection;

    /* Reflect shader program */
    //Reflect(reflection);

    /* Sort output to meet the interface requirements */
    ShaderProgram::FinalizeShaderReflection(reflection);

    return reflection;
}

void MTShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;
    
    /* Allocate new vertex descriptor */
    ReleaseVertexDesc();
    vertexDesc_ = [[MTLVertexDescriptor alloc] init];

    //todo
}

void MTShaderProgram::BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    // dummy
}

void MTShaderProgram::BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex)
{
    // dummy
}

ShaderUniform* MTShaderProgram::LockShaderUniform()
{
    return nullptr; // dummy
}

void MTShaderProgram::UnlockShaderUniform()
{
    // dummy
}


/*
 * ======= Private: =======
 */

void MTShaderProgram::ReleaseVertexDesc()
{
    if (vertexDesc_ != nullptr)
    {
        [vertexDesc_ release];
        vertexDesc_ = nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
