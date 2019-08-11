/*
 * VKShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKShader.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../../../Core/Helper.h"
#include <LLGL/ShaderProgramFlags.h>
#include <LLGL/Strings.h>

#ifdef LLGL_ENABLE_SPIRV_REFLECT
#   include "../../SPIRV/SPIRVReflect.h"
#endif


namespace LLGL
{


VKShader::VKShader(const VKPtr<VkDevice>& device, const ShaderDescriptor& desc) :
    Shader        { desc.type                     },
    device_       { device                        },
    shaderModule_ { device, vkDestroyShaderModule }
{
    Build(desc);
}

bool VKShader::HasErrors() const
{
    return (loadBinaryResult_ != LoadBinaryResult::Successful);
}

std::string VKShader::Disassemble(int flags)
{
    return ""; // dummy
}

std::string VKShader::QueryInfoLog()
{
    std::string s;

    switch (loadBinaryResult_)
    {
        case LoadBinaryResult::Undefined:
            s += ToString(GetType());
            s += " shader: shader module is undefined";
            break;
        case LoadBinaryResult::InvalidCodeSize:
            s += ToString(GetType());
            s += " shader: shader module code size is not a multiple of four bytes";
            break;
        case LoadBinaryResult::ReflectFailed:
            s = errorLog_;
            break;
        default:
            break;
    }

    return s;
}

void VKShader::FillShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& createInfo) const
{
    createInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext                = nullptr;
    createInfo.flags                = 0;
    createInfo.stage                = VKTypes::Map(GetType());
    createInfo.module               = shaderModule_;
    createInfo.pName                = entryPoint_.c_str();
    createInfo.pSpecializationInfo  = nullptr;
}

static const char* GetOptString(const char* s)
{
    return (s != nullptr ? s : "");
}

static long ShaderTypeToStageFlags(const ShaderType type)
{
    switch (type)
    {
        case ShaderType::Vertex:            return StageFlags::VertexStage;
        case ShaderType::TessControl:       return StageFlags::TessControlStage;
        case ShaderType::TessEvaluation:    return StageFlags::TessEvaluationStage;
        case ShaderType::Geometry:          return StageFlags::GeometryStage;
        case ShaderType::Fragment:          return StageFlags::FragmentStage;
        case ShaderType::Compute:           return StageFlags::ComputeStage;
        default:                            return 0;
    }
}

static ShaderResource* FindOrAppendShaderResource(ShaderReflection& reflection, const SPIRVReflect::SpvUniform& var)
{
    /* Check if there already is a resource at the specified binding slot */
    for (auto& resource : reflection.resources)
    {
        if (resource.binding.slot == var.binding)
            return &resource;
    }

    /* Append new resource entry */
    ShaderResource resource;
    {
        resource.binding.name = GetOptString(var.name);
        resource.binding.slot = var.binding;

        if (auto varType = var.type)
        {
            if (varType->opcode == spv::Op::OpTypeArray)
                resource.binding.arraySize = var.type->elements;

            if (varType->storage == spv::StorageClass::Uniform && varType->DereferenceStructPtr() != nullptr)
                resource.constantBufferSize = var.size;
        }
    }
    reflection.resources.push_back(resource);

    return &(reflection.resources.back());
}

void VKShader::Reflect(ShaderReflection& reflection) const
{
    #ifdef LLGL_ENABLE_SPIRV_REFLECT

    /* Parse shader module */
    SPIRVReflect spvReflect;
    spvReflect.Parse(shaderModuleData_.data(), shaderModuleData_.size());

    /* Gather input/output attributes */
    for (const auto& it : spvReflect.GetVaryings())
    {
        const auto& var = it.second;
        if (var.input && GetType() == ShaderType::Vertex)
        {
            VertexAttribute attrib;
            {
                attrib.name     = GetOptString(var.name);
                //attrib.format   = ;
            }
            reflection.vertexAttributes.push_back(attrib);
        }
    }

    /* Gather resources */
    for (const auto& it : spvReflect.GetUniforms())
    {
        const auto& var = it.second;
        if (auto resource = FindOrAppendShaderResource(reflection, var))
            resource->binding.stageFlags |= ShaderTypeToStageFlags(GetType());
    }

    #endif
}


/*
 * ======= Private: =======
 */

bool VKShader::Build(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(shaderDesc);
    else
        return LoadBinary(shaderDesc);
}

bool VKShader::CompileSource(const ShaderDescriptor& shaderDesc)
{
    return false; // dummy
}

bool VKShader::LoadBinary(const ShaderDescriptor& shaderDesc)
{
    /* Get shader binary */
    std::vector<char>   fileContent;
    const char*         binaryBuffer = nullptr;
    std::size_t         binaryLength = 0;

    if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
    {
        /* Load binary from file */
        fileContent = ReadFileBuffer(shaderDesc.source);
        binaryBuffer = fileContent.data();
        binaryLength = fileContent.size();
    }
    else
    {
        /* Load binary from buffer */
        binaryBuffer = shaderDesc.source;
        binaryLength = shaderDesc.sourceSize;
    }

    /* Validate code size and store data */
    if (binaryBuffer == nullptr || binaryLength % 4 != 0)
    {
        loadBinaryResult_ = LoadBinaryResult::InvalidCodeSize;
        shaderModuleData_.clear();
        return false;
    }
    else
        shaderModuleData_ = std::vector<char>(binaryBuffer, binaryBuffer + binaryLength);

    /* Store shader entry point (by default "main" for GLSL) */
    if (shaderDesc.entryPoint == nullptr || *shaderDesc.entryPoint == '\0')
        entryPoint_ = "main";
    else
        entryPoint_ = shaderDesc.entryPoint;

    /* Create shader module */
    VkShaderModuleCreateInfo createInfo;
    {
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext    = nullptr;
        createInfo.flags    = 0;
        createInfo.codeSize = binaryLength;
        createInfo.pCode    = reinterpret_cast<const std::uint32_t*>(binaryBuffer);
    }
    auto result = vkCreateShaderModule(device_, &createInfo, nullptr, shaderModule_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan shader module");

    loadBinaryResult_ = LoadBinaryResult::Successful;

    return true;
}


} // /namespace LLGL



// ================================================================================
