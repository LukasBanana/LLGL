/*
 * MTShaderProgram.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTShaderProgram.h"
#include "MTShader.h"
#include "../../CheckedCast.h"
#include "../MTTypes.h"


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
    
    /* Check for errors */
    if (shaderMT.HasError())
    {
        /* Store reference to shader with error (for later info log queries) */
        shaderWithError_ = (&shaderMT);
    }
    else
    {
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
}

void MTShaderProgram::DetachAll()
{
    shaderWithError_ = nullptr;
}

//TODO
bool MTShaderProgram::LinkShaders()
{
    return (shaderWithError_ == nullptr);
}

std::string MTShaderProgram::QueryInfoLog()
{
    if (shaderWithError_ != nullptr)
        return shaderWithError_->QueryInfoLog();
    else
        return "";
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
    
    for (std::uint32_t i = 0, attribIdx = 0; i < numVertexFormats; ++i)
    {
        auto inputSlot = vertexFormats[i].inputSlot;
    
        /* Convert vertex layout */
        MTLVertexBufferLayoutDescriptor* layoutDesc = vertexDesc_.layouts[inputSlot];
        
        //TODO: per-instance data by attribute (not by vertex format!)
        layoutDesc.stepFunction = MTLVertexStepFunctionPerVertex;
        layoutDesc.stepRate     = 1;
        layoutDesc.stride       = (NSUInteger)vertexFormats[i].stride;

        /* Convert attributes */
        const auto& attribs = vertexFormats[i].attributes;
        
        for (std::size_t j = 0; j < attribs.size(); ++j)
        {
            MTLVertexAttributeDescriptor* attribDesc = vertexDesc_.attributes[attribIdx++];
            
            attribDesc.format       = MTTypes::Map(attribs[j].vectorType);
            attribDesc.offset       = (NSUInteger)attribs[j].offset;
            attribDesc.bufferIndex  = inputSlot;
        }
    }
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
