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


MTShaderProgram::MTShaderProgram(const ShaderProgramDescriptor& desc)
{
    Attach(desc.vertexShader);
    Attach(desc.tessControlShader);
    Attach(desc.tessEvaluationShader);
    Attach(desc.geometryShader);
    Attach(desc.fragmentShader);
    Attach(desc.computeShader);
    BuildInputLayout(desc.vertexFormats.size(), desc.vertexFormats.data());
}

MTShaderProgram::~MTShaderProgram()
{
    ReleaseVertexDesc();
}

bool MTShaderProgram::HasErrors() const
{
    return (shaderWithError_ != nullptr);
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

void MTShaderProgram::Attach(Shader* shader)
{
    if (shader != nullptr)
    {
        /* Get native shader function */
        auto* shaderMT = LLGL_CAST(MTShader*, shader);
        id<MTLFunction> shaderFunc = shaderMT->GetNative();
        
        /* Check for errors */
        if (shaderMT->HasError())
        {
            /* Store reference to shader with error (for later info log queries) */
            shaderWithError_ = shaderMT;
        }
        else
        {
            /* Store reference to shader function */
            switch (shaderMT->GetType())
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
                default:
                    break;
            }
        }
    }
}

void MTShaderProgram::BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats)
{
    if (numVertexFormats == 0 || vertexFormats == nullptr)
        return;
    
    /* Allocate new vertex descriptor */
    ReleaseVertexDesc();
    vertexDesc_ = [[MTLVertexDescriptor alloc] init];
    
    for (std::size_t i = 0, attribIdx = 0; i < numVertexFormats; ++i)
    {
        auto inputSlot = vertexFormats[i].inputSlot;
    
        /* Convert vertex layout */
        MTLVertexBufferLayoutDescriptor* layoutDesc = vertexDesc_.layouts[inputSlot];
        
        //TODO: per-instance data by attribute (not by vertex format!)
        layoutDesc.stepFunction = MTLVertexStepFunctionPerVertex;
        layoutDesc.stepRate     = 1;
        layoutDesc.stride       = static_cast<NSUInteger>(vertexFormats[i].stride);

        /* Convert attributes */
        const auto& attribs = vertexFormats[i].attributes;
        
        #if 1//TODO
        if (!attribs.empty() && attribs.front().instanceDivisor > 0)
        {
            layoutDesc.stepFunction = MTLVertexStepFunctionPerInstance;
            layoutDesc.stepRate     = static_cast<NSUInteger>(attribs.front().instanceDivisor);
        }
        #endif
        
        for (std::size_t j = 0; j < attribs.size(); ++j)
        {
            MTLVertexAttributeDescriptor* attribDesc = vertexDesc_.attributes[attribIdx++];
            
            attribDesc.format       = MTTypes::ToMTLVertexFormat(attribs[j].format);
            attribDesc.offset       = static_cast<NSUInteger>(attribs[j].offset);
            attribDesc.bufferIndex  = inputSlot;
        }
    }
}

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
