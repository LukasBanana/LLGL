/*
 * VKShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
#   include "../../SPIRV/SPIRVReflectExecutionMode.h"
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

std::string VKShader::GetReport() const
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

#ifdef LLGL_ENABLE_SPIRV_REFLECT

static Format SpvVectorTypeToFormat(const SPIRVReflect::SpvType* type, std::uint32_t count)
{
    if (type->opcode == spv::Op::OpTypeFloat)
    {
        if (type->size == 16)
        {
            switch (count)
            {
                case 1: return Format::R16Float;
                case 2: return Format::RG16Float;
                case 3: return Format::RGB16Float;
                case 4: return Format::RGBA16Float;
            }
        }
        else if (type->size == 32)
        {
            switch (count)
            {
                case 1: return Format::R32Float;
                case 2: return Format::RG32Float;
                case 3: return Format::RGB32Float;
                case 4: return Format::RGBA32Float;
            }
        }
        else if (type->size == 64)
        {
            switch (count)
            {
                case 1: return Format::R64Float;
                case 2: return Format::RG64Float;
                case 3: return Format::RGB64Float;
                case 4: return Format::RGBA64Float;
            }
        }
    }
    else if (type->opcode == spv::Op::OpTypeInt)
    {
        if (type->sign)
        {
            switch (count)
            {
                case 1: return Format::R32SInt;
                case 2: return Format::RG32SInt;
                case 3: return Format::RGB32SInt;
                case 4: return Format::RGBA32SInt;
            }
        }
        else
        {
            switch (count)
            {
                case 1: return Format::R32UInt;
                case 2: return Format::RG32UInt;
                case 3: return Format::RGB32UInt;
                case 4: return Format::RGBA32UInt;
            }
        }
    }
    return Format::Undefined;
}

static Format SpvTypeToFormat(const SPIRVReflect::SpvType* type, std::uint32_t* count = nullptr)
{
    /* Return number of semantics to default value of 1 element */
    if (count != nullptr)
        *count = 1;

    if (type != nullptr)
    {
        if (type->opcode == spv::Op::OpTypePointer)
        {
            /* Dereference pointer type */
            return SpvTypeToFormat(type->baseType, count);
        }
        else if (type->opcode == spv::Op::OpTypeFloat || type->opcode == spv::Op::OpTypeInt)
        {
            /* Return format as scalar type */
            return SpvVectorTypeToFormat(type, 1);
        }
        else if (type->opcode == spv::Op::OpTypeVector)
        {
            /* Return format as vector type */
            if (type->baseType)
                return SpvVectorTypeToFormat(type->baseType, type->elements);
        }
        else if (type->opcode == spv::Op::OpTypeMatrix)
        {
            /* Return format as vector and return number of vectors */
            if (count != nullptr)
                *count = type->elements;
            return SpvTypeToFormat(type->baseType);
        }
    }

    return Format::Undefined;
}

static SystemValue SpvBuiltinToSystemValue(spv::BuiltIn type)
{
    switch (type)
    {
        case spv::BuiltIn::ClipDistance:        return SystemValue::ClipDistance;
        case spv::BuiltIn::CullDistance:        return SystemValue::CullDistance;
        //                                      return SystemValue::Color;
        case spv::BuiltIn::FragDepth:           return SystemValue::Depth;
        //                                      return SystemValue::DepthGreater;
        //                                      return SystemValue::DepthLess;
        case spv::BuiltIn::FrontFacing:         return SystemValue::FrontFacing;
        case spv::BuiltIn::InstanceId:          return SystemValue::InstanceID;
        case spv::BuiltIn::InstanceIndex:       return SystemValue::InstanceID;
        case spv::BuiltIn::Position:            return SystemValue::Position;
        case spv::BuiltIn::FragCoord:           return SystemValue::Position;
        case spv::BuiltIn::PrimitiveId:         return SystemValue::PrimitiveID;
        case spv::BuiltIn::Layer:               return SystemValue::RenderTargetIndex;
        case spv::BuiltIn::SampleMask:          return SystemValue::SampleMask;
        case spv::BuiltIn::SampleId:            return SystemValue::SampleID;
        case spv::BuiltIn::FragStencilRefEXT:   return SystemValue::Stencil;
        case spv::BuiltIn::VertexId:            return SystemValue::VertexID;
        case spv::BuiltIn::VertexIndex:         return SystemValue::VertexID;
        case spv::BuiltIn::ViewportIndex:       return SystemValue::ViewportIndex;
        default:                                return SystemValue::Undefined;
    }
}

// Reflects the SPIR-V type to the output binding descriptor and returns the dereferenced type
static const SPIRVReflect::SpvType* ReflectSpvBinding(BindingDescriptor& binding, const SPIRVReflect::SpvType* varType)
{
    if (varType != nullptr)
    {
        if (auto derefType = varType->DereferencePtr())
        {
            switch (derefType->opcode)
            {
                case spv::Op::OpTypeArray:
                    /* Multiply array in case of multiple interleaved arrays, e.g. MultiArray[4][3] is equivalent to LinearArray[4*3] */
                    binding.arraySize *= derefType->elements;
                    return ReflectSpvBinding(binding, derefType->baseType);

                case spv::Op::OpTypeImage:
                    binding.type       = ResourceType::Texture;
                    binding.bindFlags  |= BindFlags::Sampled;
                    return derefType;

                case spv::Op::OpTypeSampler:
                    binding.type       = ResourceType::Sampler;
                    return derefType;

                case spv::Op::OpTypeSampledImage:
                    binding.type       = ResourceType::Texture;
                    binding.bindFlags  |= (BindFlags::Sampled | BindFlags::CombinedTextureSampler);
                    return derefType;

                case spv::Op::OpTypeStruct:
                    binding.type       = ResourceType::Buffer;
                    binding.bindFlags  |= BindFlags::ConstantBuffer;
                    return derefType;

                default:
                    break;
            }
        }
    }
    return nullptr;
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
            //if (varType->opcode == spv::Op::OpTypeArray)
            //    resource.binding.arraySize = varType->elements;

            if (varType->storage == spv::StorageClass::Uniform ||
                varType->storage == spv::StorageClass::UniformConstant)
            {
                if (auto derefType = ReflectSpvBinding(resource.binding, varType))
                {
                    if (derefType->opcode == spv::Op::OpTypeStruct)
                        resource.constantBufferSize = var.size;
                }
            }
        }
    }
    reflection.resources.push_back(resource);

    return &(reflection.resources.back());
}

void VKShader::Reflect(ShaderReflection& reflection) const
{
    /* Parse shader module */
    SPIRVReflect spvReflect;
    spvReflect.Parse(shaderModuleData_.data(), shaderModuleData_.size());

    /* Gather input/output attributes */
    for (const auto& it : spvReflect.GetVaryings())
    {
        const auto& var = it.second;
        if (var.input && GetType() == ShaderType::Vertex)
        {
            std::uint32_t numVectors = 1;

            /* Determine vertex attribute data */
            VertexAttribute attrib;
            {
                attrib.name         = GetOptString(var.name);
                attrib.format       = SpvTypeToFormat(var.type, &numVectors);
                attrib.systemValue  = SpvBuiltinToSystemValue(var.builtin);
            }

            /* Append vertex attributes for each semantic index */
            for (std::uint32_t i = 0; i < numVectors; ++i)
            {
                attrib.semanticIndex = i;
                reflection.vertexAttributes.push_back(attrib);
            }
        }
    }

    /* Gather resources */
    for (const auto& it : spvReflect.GetUniforms())
    {
        const auto& var = it.second;
        if (auto resource = FindOrAppendShaderResource(reflection, var))
            resource->binding.stageFlags |= ShaderTypeToStageFlags(GetType());
    }
}

bool VKShader::ReflectLocalSize(Extent3D& localSize) const
{
    if (GetType() == ShaderType::Compute)
    {
        /* Parse shader module */
        SPIRVReflectExecutionMode spvReflect;
        spvReflect.Parse(shaderModuleData_.data(), shaderModuleData_.size());

        /* Return local work group size */
        const auto& mode = spvReflect.GetMode();
        localSize.width     = mode.localSizeX;
        localSize.height    = mode.localSizeY;
        localSize.depth     = mode.localSizeZ;

        return true;
    }
    return false;
}

#else

void VKShader::Reflect(ShaderReflection& /*reflection*/) const
{
    // dummy
}

bool VKShader::ReflectLocalSize(Extent3D& workGroupSize) const
{
    return false; // dummy
}

#endif // /LLGL_ENABLE_SPIRV_REFLECT


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
