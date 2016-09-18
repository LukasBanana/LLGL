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
    if (!shaderDbg.IsCompiled())
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "attempt to attach uncompiled shader to shader program");
    instance.AttachShader(shaderDbg.instance);
}

bool DbgShaderProgram::LinkShaders()
{
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

std::vector<ConstantBufferDescriptor> DbgShaderProgram::QueryConstantBuffers() const
{
    return instance.QueryConstantBuffers();
}

std::vector<StorageBufferDescriptor> DbgShaderProgram::QueryStorageBuffers() const
{
    return instance.QueryStorageBuffers();
}

std::vector<UniformDescriptor> DbgShaderProgram::QueryUniforms() const
{
    return instance.QueryUniforms();
}

void DbgShaderProgram::BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs)
{
    instance.BindVertexAttributes(vertexAttribs);
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


} // /namespace LLGL



// ================================================================================
