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
    BuildShader(desc);
    BuildInputLayout(desc.vertex.inputAttribs.size(), desc.vertex.inputAttribs.data());
}

bool VKShader::HasErrors() const
{
    return (loadBinaryResult_ != LoadBinaryResult::Successful);
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

void VKShader::FillVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& createInfo) const
{
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    if (inputLayout_.bindingDescs.empty())
    {
        createInfo.vertexBindingDescriptionCount    = 0;
        createInfo.pVertexBindingDescriptions       = nullptr;
    }
    else
    {
        createInfo.vertexBindingDescriptionCount    = static_cast<std::uint32_t>(inputLayout_.bindingDescs.size());
        createInfo.pVertexBindingDescriptions       = inputLayout_.bindingDescs.data();
    }

    if (inputLayout_.attribDescs.empty())
    {
        createInfo.vertexAttributeDescriptionCount  = 0;
        createInfo.pVertexAttributeDescriptions     = nullptr;
    }
    else
    {
        createInfo.vertexAttributeDescriptionCount  = static_cast<std::uint32_t>(inputLayout_.attribDescs.size());
        createInfo.pVertexAttributeDescriptions     = inputLayout_.attribDescs.data();
    }
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
                    binding.bindFlags  |= (BindFlags::Sampled | BindFlags::CombinedSampler);
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

bool VKShader::Reflect(ShaderReflection& reflection) const
{
    /* Parse shader module */
    SPIRVReflect spvReflect;
    spvReflect.Parse(shaderModuleData_.data(), shaderModuleData_.size());

    /* Gather input/output attributes */
    for (const auto& it : spvReflect.GetVaryings())
    {
        const auto& var = it.second;
        if (GetType() == ShaderType::Vertex)
        {
            std::uint32_t numVectors = 1;

            /* Determine vertex attribute data */
            VertexAttribute attrib;
            {
                attrib.name         = GetOptString(var.name);
                attrib.format       = SpvTypeToFormat(var.type, &numVectors);
                attrib.location     = var.location;
                attrib.systemValue  = SpvBuiltinToSystemValue(var.builtin);
            }

            /* Append vertex attributes for each semantic index */
            for (std::uint32_t i = 0; i < numVectors; ++i)
            {
                attrib.semanticIndex = i;
                if (var.input)
                    reflection.vertex.inputAttribs.push_back(attrib);
                else
                    reflection.vertex.outputAttribs.push_back(attrib);
            }
        }
        else if (GetType() == ShaderType::Fragment && !var.input)
        {
            /* Determine and append fragment attribute data */
            FragmentAttribute attrib;
            {
                attrib.name         = GetOptString(var.name);
                attrib.format       = SpvTypeToFormat(var.type);
                attrib.location     = var.location;
                attrib.systemValue  = SpvBuiltinToSystemValue(var.builtin);
            }
            reflection.fragment.outputAttribs.push_back(attrib);
        }
    }

    /* Gather resources */
    for (const auto& it : spvReflect.GetUniforms())
    {
        const auto& var = it.second;
        if (auto resource = FindOrAppendShaderResource(reflection, var))
            resource->binding.stageFlags |= ShaderTypeToStageFlags(GetType());
    }

    return true;
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

bool VKShader::Reflect(ShaderReflection& /*reflection*/) const
{
    return false; // dummy
}

bool VKShader::ReflectLocalSize(Extent3D& /*workGroupSize*/) const
{
    return false; // dummy
}

#endif // /LLGL_ENABLE_SPIRV_REFLECT


/*
 * ======= Private: =======
 */

bool VKShader::BuildShader(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(shaderDesc);
    else
        return LoadBinary(shaderDesc);
}

// Helper structure to build set of <VkVertexInputBindingDescription> elements
struct VKCompareVertexBindingDesc
{
    inline bool operator () (const VkVertexInputBindingDescription& lhs, const VkVertexInputBindingDescription& rhs) const
    {
        return (lhs.binding < rhs.binding);
    }
};

void VKShader::BuildInputLayout(std::size_t numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    if (numVertexAttribs == 0 || vertexAttribs == nullptr)
        return;

    inputLayout_.bindingDescs.reserve(numVertexAttribs);

    std::set<VkVertexInputBindingDescription, VKCompareVertexBindingDesc> bindingDescSet;

    for (std::size_t i = 0; i < numVertexAttribs; ++i)
    {
        const auto& attr = vertexAttribs[i];

        if (attr.instanceDivisor > 1)
        {
            throw std::runtime_error(
                "vertex instance divisor must be 0 or 1 for Vulkan, but " +
                std::to_string(attr.instanceDivisor) + " was specified: " + attr.name
            );
        }

        /* Append vertex input attribute descriptor */
        VkVertexInputAttributeDescription vertexAttrib;
        {
            vertexAttrib.location   = attr.location;
            vertexAttrib.binding    = attr.slot;
            vertexAttrib.format     = VKTypes::Map(attr.format);
            vertexAttrib.offset     = attr.offset;
        }
        inputLayout_.attribDescs.push_back(vertexAttrib);

        /* Insert vertex binding descriptor */
        VkVertexInputBindingDescription inputBinding;
        {
            inputBinding.binding    = attr.slot;
            inputBinding.stride     = attr.stride;
            inputBinding.inputRate  = (attr.instanceDivisor > 0 ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX);
        }
        bindingDescSet.insert(inputBinding);
    }

    /* Store binding descriptor in vector */
    inputLayout_.bindingDescs.insert(inputLayout_.bindingDescs.end(), bindingDescSet.begin(), bindingDescSet.end());
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
