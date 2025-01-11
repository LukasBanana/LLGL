/*
 * VKShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKShader.h"
#include "VKShaderModulePool.h"
#include "../VKCore.h"
#include "../VKTypes.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/StringUtils.h"
#include "../../../Core/ReportUtils.h"
#include "../../../Core/Assertion.h"
#include "../../PipelineStateUtils.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include <string.h>
#include <algorithm>
#include <set>

#if LLGL_VK_ENABLE_SPIRV_REFLECT
#   include "../../SPIRV/SpirvReflect.h"
#endif


namespace LLGL
{


static VKPtr<VkShaderModule> CreateVkShaderModule(VkDevice device, const std::vector<std::uint32_t>& shaderCode)
{
    VkShaderModuleCreateInfo createInfo;
    {
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext    = nullptr;
        createInfo.flags    = 0;
        createInfo.codeSize = shaderCode.size() * sizeof(std::uint32_t);
        createInfo.pCode    = shaderCode.data();
    }
    VKPtr<VkShaderModule> shaderModule{ device, vkDestroyShaderModule };
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, shaderModule.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan shader module");
    return shaderModule;
}

VKShader::VKShader(VkDevice device, const ShaderDescriptor& desc) :
    Shader  { desc.type },
    device_ { device    }
{
    BuildShader(desc);
    BuildInputLayout(desc.vertex.inputAttribs.size(), desc.vertex.inputAttribs.data());
    BuildBindingLayout();
    BuildReport();
}

VKShader::~VKShader()
{
    VKShaderModulePool::Get().NotifyReleaseShader(this);
}

const Report* VKShader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
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

bool VKShader::NeedsShaderModulePermutation(const PermutationBindingFunc& permutationBindingFunc) const
{
    if (!permutationBindingFunc)
        return false;

    /* Re-assign binding slots with a permutation of the binding layout */
    ConstFieldRangeIterator<BindingSlot> bindingSlotIter;
    std::uint32_t dstSet;

    for (unsigned index = 0; permutationBindingFunc(index, bindingSlotIter, dstSet); ++index)
    {
        if (!bindingLayout_.MatchesBindingSlots(bindingSlotIter, dstSet))
            return true;
    }

    return false;
}

VKPtr<VkShaderModule> VKShader::CreateVkShaderModulePermutation(const PermutationBindingFunc& permutationBindingFunc)
{
    if (!permutationBindingFunc)
        return VK_NULL_HANDLE;

    /* Re-assign binding slots with a permutation of the binding layout */
    VKShaderBindingLayout bindingLayoutPerm = bindingLayout_;

    ConstFieldRangeIterator<BindingSlot> bindingSlotIter;
    std::uint32_t dstSet;
    bool modified = false;

    for (unsigned index = 0; permutationBindingFunc(index, bindingSlotIter, dstSet); ++index)
    {
        if (bindingLayoutPerm.AssignBindingSlots(bindingSlotIter, dstSet) > 0)
            modified = true;
    }

    /* Create shader module permuation if there is at least one modified binding slot */
    if (modified)
    {
        VKShaderCode shaderCodePerm = shaderCode_;
        bindingLayoutPerm.UpdateSpirvModule(shaderCodePerm.data(), shaderCodePerm.size() * sizeof(std::uint32_t));
        return CreateVkShaderModule(device_, shaderCodePerm);
    }

    return VK_NULL_HANDLE;
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

#if LLGL_VK_ENABLE_SPIRV_REFLECT

static Format SpvVectorTypeToFormat(const SpirvReflect::SpvType* type, std::uint32_t count)
{
    if (type->opcode == spv::Op::OpTypeFloat)
    {
        if (type->size == 2)
        {
            switch (count)
            {
                case 1: return Format::R16Float;
                case 2: return Format::RG16Float;
                case 3: return Format::RGB16Float;
                case 4: return Format::RGBA16Float;
            }
        }
        else if (type->size == 4)
        {
            switch (count)
            {
                case 1: return Format::R32Float;
                case 2: return Format::RG32Float;
                case 3: return Format::RGB32Float;
                case 4: return Format::RGBA32Float;
            }
        }
        else if (type->size == 8)
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

static Format SpvTypeToFormat(const SpirvReflect::SpvType* type, std::uint32_t* count = nullptr)
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
        case spv::BuiltInClipDistance:      return SystemValue::ClipDistance;
        case spv::BuiltInCullDistance:      return SystemValue::CullDistance;
        //                                  return SystemValue::Color;
        case spv::BuiltInFragDepth:         return SystemValue::Depth;
        //                                  return SystemValue::DepthGreater;
        //                                  return SystemValue::DepthLess;
        case spv::BuiltInFrontFacing:       return SystemValue::FrontFacing;
        case spv::BuiltInInstanceId:        return SystemValue::InstanceID;
        case spv::BuiltInInstanceIndex:     return SystemValue::InstanceID;
        case spv::BuiltInPosition:          return SystemValue::Position;
        case spv::BuiltInFragCoord:         return SystemValue::Position;
        case spv::BuiltInPrimitiveId:       return SystemValue::PrimitiveID;
        case spv::BuiltInLayer:             return SystemValue::RenderTargetIndex;
        case spv::BuiltInSampleMask:        return SystemValue::SampleMask;
        case spv::BuiltInSampleId:          return SystemValue::SampleID;
        case spv::BuiltInFragStencilRefEXT: return SystemValue::Stencil;
        case spv::BuiltInVertexId:          return SystemValue::VertexID;
        case spv::BuiltInVertexIndex:       return SystemValue::VertexID;
        case spv::BuiltInViewportIndex:     return SystemValue::ViewportIndex;
        default:                            return SystemValue::Undefined;
    }
}

static SystemValue SpvBuiltinToFragmentOutputSV(spv::BuiltIn type)
{
    SystemValue sv = SpvBuiltinToSystemValue(type);
    return (sv == SystemValue::Undefined ? SystemValue::Color : sv);
}

// Reflects the SPIR-V type to the output binding descriptor and returns the dereferenced type
static const SpirvReflect::SpvType* ReflectSpvBinding(BindingDescriptor& binding, const SpirvReflect::SpvType* varType)
{
    if (varType != nullptr)
    {
        if (const SpirvReflect::SpvType* derefType = varType->Deref())
        {
            switch (derefType->opcode)
            {
                case spv::Op::OpTypeArray:
                    /* Multiply array in case of multiple interleaved arrays, e.g. MultiArray[4][3] is equivalent to LinearArray[4*3] */
                    binding.arraySize = (derefType->elements == 0 ? derefType->elements : binding.arraySize * derefType->elements);
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

static ShaderResourceReflection* FindOrAppendShaderResource(ShaderReflection& reflection, const SpirvReflect::SpvUniform& var)
{
    /* Check if there already is a resource at the specified binding slot */
    for (ShaderResourceReflection& resource : reflection.resources)
    {
        if (resource.binding.slot == BindingSlot{ var.binding, var.set })
            return &resource;
    }

    /* Append new resource entry */
    ShaderResourceReflection resource;
    {
        resource.binding.name = GetOptString(var.name);
        resource.binding.slot = var.binding;

        if (const SpirvReflect::SpvType* varType = var.type)
        {
            //if (varType->opcode == spv::Op::OpTypeArray)
            //    resource.binding.arraySize = varType->elements;

            if (varType->storage == spv::StorageClassUniform ||
                varType->storage == spv::StorageClassUniformConstant)
            {
                if (const SpirvReflect::SpvType* derefType = ReflectSpvBinding(resource.binding, varType))
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

static UniformType ReflectUniformType(const SpirvReflect::SpvType* type)
{
    if (type != nullptr)
    {
        switch (type->opcode)
        {
            case spv::OpTypeArray:
                /* Just dereference type since array elements are handled outside this function */
                return ReflectUniformType(type->baseType);

            case spv::OpTypeMatrix:
                return MakeUniformMatrixType(ReflectUniformType(type->baseType), type->elements);

            case spv::OpTypeVector:
                return MakeUniformVectorType(ReflectUniformType(type->baseType), type->elements);

            case spv::OpTypeFloat:
                return (type->size == 2 ? UniformType::Double1 : UniformType::Float1);

            case spv::OpTypeInt:
                return (type->sign ? UniformType::Int1 : UniformType::UInt1);

            case spv::OpTypeBool:
                return UniformType::Bool1;

            default:
                break;
        }
    }
    return UniformType::Undefined;
}

bool VKShader::Reflect(ShaderReflection& reflection) const
{
    /* Parse shader module */
    SpirvReflect spvReflect;
    if (spvReflect.Reflect(SpirvModuleView{ shaderCode_ }) != SpirvResult::NoError)
        return false;

    /* Gather input/output attributes */
    for (const auto& it : spvReflect.GetVaryings())
    {
        const SpirvReflect::SpvVarying& var = it.second;
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
            for_range(i, numVectors)
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
                attrib.systemValue  = SpvBuiltinToFragmentOutputSV(var.builtin);
            }
            reflection.fragment.outputAttribs.push_back(attrib);
        }
    }

    /* Gather shader resources */
    for (const auto& it : spvReflect.GetUniforms())
    {
        const SpirvReflect::SpvUniform& var = it.second;
        if (ShaderResourceReflection* resource = FindOrAppendShaderResource(reflection, var))
            resource->binding.stageFlags |= ShaderTypeToStageFlags(GetType());
    }

    /* Gather push constants */
    if (const SpirvReflect::SpvType* pushConstantType = spvReflect.GetPushConstantStructType())
    {
        reflection.uniforms.reserve(reflection.uniforms.size() + pushConstantType->fieldTypes.size());
        if (pushConstantType->fieldTypes.size() == pushConstantType->fieldNames.size())
        {
            for_range(i, pushConstantType->fieldTypes.size())
            {
                const SpirvReflect::SpvType* fieldType = pushConstantType->fieldTypes[i];
                const char* fieldName = pushConstantType->fieldNames[i];

                UniformDescriptor uniformDesc;
                {
                    uniformDesc.name        = fieldName;
                    uniformDesc.type        = ReflectUniformType(fieldType);
                    uniformDesc.arraySize   = (fieldType->opcode == spv::OpTypeArray ? fieldType->elements : 0);
                }
                reflection.uniforms.push_back(uniformDesc);
            }
        }
    }

    return true;
}

bool VKShader::ReflectLocalSize(Extent3D& outLocalSize) const
{
    if (GetType() != ShaderType::Compute)
        return false;

    /* Parse shader module for execution mode */
    SpirvReflect::SpvExecutionMode executionMode;
    SpirvReflectExecutionMode(SpirvModuleView{ shaderCode_ }, executionMode);

    /* Return local work group size */
    outLocalSize.width  = executionMode.localSizeX;
    outLocalSize.height = executionMode.localSizeY;
    outLocalSize.depth  = executionMode.localSizeZ;

    return true;
}

bool VKShader::ReflectPushConstants(
    const ArrayView<UniformDescriptor>& inUniformDescs,
    std::vector<VKUniformRange>&        outUniformRanges) const
{
    /* Initialize output container with zero-ranges */
    outUniformRanges.resize(inUniformDescs.size());

    /* Parse shader module for push-constants */
    SpirvReflect::SpvBlock block;
    SpirvResult result = SpirvReflectPushConstants(SpirvModuleView{ shaderCode_ }, block);
    if (result != SpirvResult::NoError)
        return false;

    /* Build push constant ranges */
    for_range(i, inUniformDescs.size())
    {
        /* Find name of uniform descriptor in push-constant block fields */
        const UniformDescriptor& uniformDesc = inUniformDescs[i];
        for (const SpirvReflect::SpvBlockField& field : block.fields)
        {
            if (field.name != nullptr && ::strcmp(field.name, uniformDesc.name.c_str()) == 0)
            {
                VKUniformRange& range = outUniformRanges[i];
                {
                    range.offset    = field.offset;
                    range.size      = GetUniformTypeSize(uniformDesc.type, uniformDesc.arraySize);
                }
                break;
            }
        }
    }

    return true;
}

#else // LLGL_VK_ENABLE_SPIRV_REFLECT

bool VKShader::Reflect(ShaderReflection& /*reflection*/) const
{
    return false; // dummy
}

bool VKShader::ReflectLocalSize(Extent3D& /*outLocalSize*/) const
{
    return false; // dummy
}

bool VKShader::ReflectPushConstants(
    const ArrayView<UniformDescriptor>& inUniformDescs,
    std::vector<VKUniformRange>&        outUniformRanges) const
{
    return false; // dummy
}

#endif // /LLGL_VK_ENABLE_SPIRV_REFLECT


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

    for_range(i, numVertexAttribs)
    {
        const VertexAttribute& attr = vertexAttribs[i];

        LLGL_ASSERT(
            !(attr.instanceDivisor > 1),
            "vertex instance divisor must be 0 or 1 for Vulkan, but %u was specified: %s",
            attr.instanceDivisor, attr.name.c_str()
        );

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

void VKShader::BuildBindingLayout()
{
    bindingLayout_.BuildFromSpirvModule(shaderCode_.data(), shaderCode_.size() * sizeof(std::uint32_t));
}

void VKShader::BuildReport()
{
    switch (loadBinaryResult_)
    {
        case LoadBinaryResult::Undefined:
            report_.Errorf("%s shader: shader module is undefined\n", ToString(GetType()));
            break;
        case LoadBinaryResult::InvalidCodeSize:
            report_.Errorf("%s shader: shader module code size is not a multiple of four bytes\n", ToString(GetType()));
            break;
        case LoadBinaryResult::ReflectFailed:
            //TODO
            break;
        default:
            break;
    }
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
        shaderCode_.clear();
        return false;
    }
    else
    {
        const std::uint32_t* words = reinterpret_cast<const std::uint32_t*>(binaryBuffer);
        shaderCode_ = std::vector<std::uint32_t>(words, words + binaryLength/sizeof(std::uint32_t));
    }

    /* Store shader entry point (by default "main" for GLSL) */
    if (shaderDesc.entryPoint == nullptr || *shaderDesc.entryPoint == '\0')
        entryPoint_ = "main";
    else
        entryPoint_ = shaderDesc.entryPoint;

    /* Create shader module */
    shaderModule_ = CreateVkShaderModule(device_, shaderCode_);

    loadBinaryResult_ = LoadBinaryResult::Successful;

    return true;
}


} // /namespace LLGL



// ================================================================================
