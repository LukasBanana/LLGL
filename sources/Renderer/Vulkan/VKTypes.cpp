/*
 * VKTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace VKTypes
{


/* ----- Map functions ----- */

[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& vknTypeName)
{
    throw std::invalid_argument("failed to map <LLGL::" + typeName + "> to <" + vknTypeName + "> Vulkan parameter");
}

VkShaderStageFlagBits Map(const ShaderType shaderType)
{
    switch (shaderType)
    {
        case ShaderType::Vertex:            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderType::TessControl:       return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderType::TessEvaluation:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderType::Geometry:          return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderType::Fragment:          return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderType::Compute:           return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    MapFailed("ShaderType", "VkShaderStageFlagBits");
}

VkFormat Map(const Format format)
{
    switch (format)
    {
        case Format::Undefined:         return VK_FORMAT_UNDEFINED;

        /* --- Alpha channel color formats --- */
        case Format::A8UNorm:           break;

        /* --- Red channel color formats --- */
        case Format::R8UNorm:           return VK_FORMAT_R8_UNORM;
        case Format::R8SNorm:           return VK_FORMAT_R8_SNORM;
        case Format::R8UInt:            return VK_FORMAT_R8_UINT;
        case Format::R8SInt:            return VK_FORMAT_R8_SINT;

        case Format::R16UNorm:          return VK_FORMAT_R16_UNORM;
        case Format::R16SNorm:          return VK_FORMAT_R16_SNORM;
        case Format::R16UInt:           return VK_FORMAT_R16_UINT;
        case Format::R16SInt:           return VK_FORMAT_R16_SINT;
        case Format::R16Float:          return VK_FORMAT_R16_SFLOAT;

        case Format::R32UInt:           return VK_FORMAT_R32_UINT;
        case Format::R32SInt:           return VK_FORMAT_R32_SINT;
        case Format::R32Float:          return VK_FORMAT_R32_SFLOAT;

        case Format::R64Float:          return VK_FORMAT_R64_SFLOAT;

        /* --- RG color formats --- */
        case Format::RG8UNorm:          return VK_FORMAT_R8G8_UNORM;
        case Format::RG8SNorm:          return VK_FORMAT_R8G8_SNORM;
        case Format::RG8UInt:           return VK_FORMAT_R8G8_UINT;
        case Format::RG8SInt:           return VK_FORMAT_R8G8_SINT;

        case Format::RG16UNorm:         return VK_FORMAT_R16G16_UNORM;
        case Format::RG16SNorm:         return VK_FORMAT_R16G16_SNORM;
        case Format::RG16UInt:          return VK_FORMAT_R16G16_UINT;
        case Format::RG16SInt:          return VK_FORMAT_R16G16_SINT;
        case Format::RG16Float:         return VK_FORMAT_R16G16_SFLOAT;

        case Format::RG32UInt:          return VK_FORMAT_R32G32_UINT;
        case Format::RG32SInt:          return VK_FORMAT_R32G32_SINT;
        case Format::RG32Float:         return VK_FORMAT_R32G32_SFLOAT;

        case Format::RG64Float:         return VK_FORMAT_R64G64_SFLOAT;

        /* --- RGB color formats --- */
        case Format::RGB8UNorm:         return VK_FORMAT_R8G8B8_UNORM;
        case Format::RGB8UNorm_sRGB:    return VK_FORMAT_R8G8B8_SRGB;
        case Format::RGB8SNorm:         return VK_FORMAT_R8G8B8_SNORM;
        case Format::RGB8UInt:          return VK_FORMAT_R8G8B8_UINT;
        case Format::RGB8SInt:          return VK_FORMAT_R8G8B8_SINT;

        case Format::RGB16UNorm:        return VK_FORMAT_R16G16B16_UNORM;
        case Format::RGB16SNorm:        return VK_FORMAT_R16G16B16_SNORM;
        case Format::RGB16UInt:         return VK_FORMAT_R16G16B16_UINT;
        case Format::RGB16SInt:         return VK_FORMAT_R16G16B16_SINT;
        case Format::RGB16Float:        return VK_FORMAT_R16G16B16_SFLOAT;

        case Format::RGB32UInt:         return VK_FORMAT_R32G32B32_UINT;
        case Format::RGB32SInt:         return VK_FORMAT_R32G32B32_SINT;
        case Format::RGB32Float:        return VK_FORMAT_R32G32B32_SFLOAT;

        case Format::RGB64Float:        return VK_FORMAT_R64G64B64_SFLOAT;

        /* --- RGBA color formats --- */
        case Format::RGBA8UNorm:        return VK_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8UNorm_sRGB:   return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::RGBA8SNorm:        return VK_FORMAT_R8G8B8A8_SNORM;
        case Format::RGBA8UInt:         return VK_FORMAT_R8G8B8A8_UINT;
        case Format::RGBA8SInt:         return VK_FORMAT_R8G8B8A8_SINT;

        case Format::RGBA16UNorm:       return VK_FORMAT_R16G16B16A16_UNORM;
        case Format::RGBA16SNorm:       return VK_FORMAT_R16G16B16A16_SNORM;
        case Format::RGBA16UInt:        return VK_FORMAT_R16G16B16A16_UINT;
        case Format::RGBA16SInt:        return VK_FORMAT_R16G16B16A16_SINT;
        case Format::RGBA16Float:       return VK_FORMAT_R16G16B16A16_SFLOAT;

        case Format::RGBA32UInt:        return VK_FORMAT_R32G32B32A32_UINT;
        case Format::RGBA32SInt:        return VK_FORMAT_R32G32B32A32_SINT;
        case Format::RGBA32Float:       return VK_FORMAT_R32G32B32A32_SFLOAT;

        case Format::RGBA64Float:       return VK_FORMAT_R64G64B64A64_SFLOAT;

        /* --- BGRA color formats --- */
        case Format::BGRA8UNorm:        return VK_FORMAT_B8G8R8A8_UNORM;
        case Format::BGRA8UNorm_sRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::BGRA8SNorm:        return VK_FORMAT_B8G8R8A8_SNORM;
        case Format::BGRA8UInt:         return VK_FORMAT_B8G8R8A8_UINT;
        case Format::BGRA8SInt:         return VK_FORMAT_B8G8R8A8_SINT;

        /* --- Packed formats --- */
        case Format::RGB10A2UNorm:      return VK_FORMAT_A2B10G10R10_UNORM_PACK32;  // requires swizzling for CPU access
        case Format::RGB10A2UInt:       return VK_FORMAT_A2B10G10R10_UINT_PACK32;   // requires swizzling for CPU access
        case Format::RG11B10Float:      return VK_FORMAT_B10G11R11_UFLOAT_PACK32;   // requires swizzling for CPU access
        case Format::RGB9E5Float:       return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;    // requires swizzling for CPU access

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          return VK_FORMAT_D16_UNORM;
        case Format::D32Float:          return VK_FORMAT_D32_SFLOAT;
        case Format::D24UNormS8UInt:    return VK_FORMAT_D24_UNORM_S8_UINT;
        case Format::D32FloatS8X24UInt: return VK_FORMAT_D32_SFLOAT_S8_UINT;

        /* --- Block compression (BC) formats --- */
        case Format::BC1UNorm:          return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case Format::BC1UNorm_sRGB:     return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case Format::BC2UNorm:          return VK_FORMAT_BC2_UNORM_BLOCK;
        case Format::BC2UNorm_sRGB:     return VK_FORMAT_BC2_SRGB_BLOCK;
        case Format::BC3UNorm:          return VK_FORMAT_BC3_UNORM_BLOCK;
        case Format::BC3UNorm_sRGB:     return VK_FORMAT_BC3_SRGB_BLOCK;
        case Format::BC4UNorm:          return VK_FORMAT_BC4_UNORM_BLOCK;
        case Format::BC4SNorm:          return VK_FORMAT_BC4_SNORM_BLOCK;
        case Format::BC5UNorm:          return VK_FORMAT_BC5_UNORM_BLOCK;
        case Format::BC5SNorm:          return VK_FORMAT_BC5_SNORM_BLOCK;
    }
    MapFailed("Format", "VkFormat");
}

VkImageViewType Map(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return VK_IMAGE_VIEW_TYPE_1D;
        case TextureType::Texture2D:        return VK_IMAGE_VIEW_TYPE_2D;
        case TextureType::Texture3D:        return VK_IMAGE_VIEW_TYPE_3D;
        case TextureType::TextureCube:      return VK_IMAGE_VIEW_TYPE_CUBE;
        case TextureType::Texture1DArray:   return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        case TextureType::Texture2DArray:   return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case TextureType::TextureCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case TextureType::Texture2DMS:      return VK_IMAGE_VIEW_TYPE_2D;
        case TextureType::Texture2DMSArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    MapFailed("TextureType", "VkImageViewType");
}

VkPrimitiveTopology Map(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:              return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:               return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:              return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::LineLoop:               break;
        case PrimitiveTopology::LineListAdjacency:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::LineStripAdjacency:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleList:           return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip:          return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveTopology::TriangleListAdjacency:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        default:
            if (primitiveTopology >= PrimitiveTopology::Patches1 && primitiveTopology <= PrimitiveTopology::Patches32)
                return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
    }
    MapFailed("PrimitiveTopology", "VkPrimitiveTopology");
}

VkPolygonMode Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return VK_POLYGON_MODE_FILL;
        case PolygonMode::Wireframe:    return VK_POLYGON_MODE_LINE;
        case PolygonMode::Points:       return VK_POLYGON_MODE_POINT;
    }
    MapFailed("PolygonMode", "VkPolygonMode");
}

VkCullModeFlags Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return VK_CULL_MODE_NONE;
        case CullMode::Front:       return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:        return VK_CULL_MODE_BACK_BIT;
    }
    MapFailed("CullMode", "VkCullModeFlags");
}

VkCompareOp Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::NeverPass:      return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:           return VK_COMPARE_OP_LESS;
        case CompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessEqual:      return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterEqual:   return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::AlwaysPass:     return VK_COMPARE_OP_ALWAYS;
    }
    MapFailed("CompareOp", "VkCompareOp");
}

VkStencilOp Map(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return VK_STENCIL_OP_KEEP;
        case StencilOp::Zero:       return VK_STENCIL_OP_ZERO;
        case StencilOp::Replace:    return VK_STENCIL_OP_REPLACE;
        case StencilOp::IncClamp:   return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DecClamp:   return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::Invert:     return VK_STENCIL_OP_INVERT;
        case StencilOp::IncWrap:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::DecWrap:    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }
    MapFailed("StencilOp", "VkStencilOp");
}

VkLogicOp Map(const LogicOp logicOp)
{
    switch (logicOp)
    {
        case LogicOp::Disabled:     break;
        case LogicOp::Clear:        return VK_LOGIC_OP_CLEAR;
        case LogicOp::Set:          return VK_LOGIC_OP_SET;
        case LogicOp::Copy:         return VK_LOGIC_OP_COPY;
        case LogicOp::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOp::NoOp:         return VK_LOGIC_OP_NO_OP;
        case LogicOp::Invert:       return VK_LOGIC_OP_INVERT;
        case LogicOp::AND:          return VK_LOGIC_OP_AND;
        case LogicOp::ANDReverse:   return VK_LOGIC_OP_AND_REVERSE;
        case LogicOp::ANDInverted:  return VK_LOGIC_OP_AND_INVERTED;
        case LogicOp::NAND:         return VK_LOGIC_OP_NAND;
        case LogicOp::OR:           return VK_LOGIC_OP_OR;
        case LogicOp::ORReverse:    return VK_LOGIC_OP_OR_REVERSE;
        case LogicOp::ORInverted:   return VK_LOGIC_OP_OR_INVERTED;
        case LogicOp::NOR:          return VK_LOGIC_OP_NOR;
        case LogicOp::XOR:          return VK_LOGIC_OP_XOR;
        case LogicOp::Equiv:        return VK_LOGIC_OP_EQUIVALENT;
    }
    MapFailed("LogicOp", "VkLogicOp");
}

VkBlendFactor Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return VK_BLEND_FACTOR_ZERO;
        case BlendOp::One:              return VK_BLEND_FACTOR_ONE;
        case BlendOp::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendOp::InvSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendOp::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendOp::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
        case BlendOp::InvDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendOp::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendOp::InvDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendOp::BlendFactor:      return VK_BLEND_FACTOR_CONSTANT_COLOR;              // VK_BLEND_FACTOR_CONSTANT_ALPHA ???
        case BlendOp::InvBlendFactor:   return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;    // VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA ???
        case BlendOp::Src1Color:        return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    }
    MapFailed("BlendOp", "VkBlendFactor");
}

VkBlendOp Map(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return VK_BLEND_OP_ADD;
        case BlendArithmetic::Subtract:     return VK_BLEND_OP_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendArithmetic::Min:          return VK_BLEND_OP_MIN;
        case BlendArithmetic::Max:          return VK_BLEND_OP_MAX;
    }
    MapFailed("BlendArithmetic", "VkBlendOp");
}

VkSamplerAddressMode Map(const SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case SamplerAddressMode::Repeat:        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::Mirror:        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerAddressMode::Clamp:         return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::Border:        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerAddressMode::MirrorOnce:    return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }
    MapFailed("SamplerAddressMode", "VkSamplerAddressMode");
}

VkQueryType Map(const QueryType queryType)
{
    switch (queryType)
    {
        case QueryType::SamplesPassed:                  /* pass */
        case QueryType::AnySamplesPassed:               /* pass */
        case QueryType::AnySamplesPassedConservative:   return VK_QUERY_TYPE_OCCLUSION;
        case QueryType::TimeElapsed:                    return VK_QUERY_TYPE_TIMESTAMP;
        case QueryType::StreamOutPrimitivesWritten:     break;
        case QueryType::StreamOutOverflow:              break;
        case QueryType::PipelineStatistics:             return VK_QUERY_TYPE_PIPELINE_STATISTICS;
    }
    MapFailed("QueryType", "VkQueryType");
}

VkAttachmentLoadOp Map(const AttachmentLoadOp loadOp)
{
    switch (loadOp)
    {
        case AttachmentLoadOp::Undefined:   return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case AttachmentLoadOp::Load:        return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear:       return VK_ATTACHMENT_LOAD_OP_CLEAR;
    }
    MapFailed("AttachmentLoadOp", "VkAttachmentLoadOp");
}

VkAttachmentStoreOp Map(const AttachmentStoreOp storeOp)
{
    switch (storeOp)
    {
        case AttachmentStoreOp::Undefined:  return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case AttachmentStoreOp::Store:      return VK_ATTACHMENT_STORE_OP_STORE;
    }
    MapFailed("AttachmentStoreOp", "VkAttachmentStoreOp");
}

VkStencilFaceFlags Map(const StencilFace stencilFace)
{
    switch (stencilFace)
    {
        case StencilFace::FrontAndBack: return VK_STENCIL_FRONT_AND_BACK;
        case StencilFace::Front:        return VK_STENCIL_FACE_FRONT_BIT;
        case StencilFace::Back:         return VK_STENCIL_FACE_BACK_BIT;
    }
    MapFailed("StencilFace", "VkStencilFaceFlags");
}

VkPipelineBindPoint Map(const PipelineBindPoint pipelineBindPoint)
{
    switch (pipelineBindPoint)
    {
        case PipelineBindPoint::Undefined:  return VK_PIPELINE_BIND_POINT_MAX_ENUM;
        case PipelineBindPoint::Graphics:   return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case PipelineBindPoint::Compute:    return VK_PIPELINE_BIND_POINT_COMPUTE;
    }
    VKTypes::MapFailed("PipelineBindPoint", "VkPipelineBindPoint");
}

VkIndexType ToVkIndexType(const Format format)
{
    switch (format)
    {
        case Format::Undefined: return VK_INDEX_TYPE_MAX_ENUM;
        case Format::R16UInt:   return VK_INDEX_TYPE_UINT16;
        case Format::R32UInt:   return VK_INDEX_TYPE_UINT32;
        default:                break;
    }
    VKTypes::MapFailed("Format", "VkIndexType");
}

VkSampleCountFlagBits ToVkSampleCountBits(std::uint32_t samples)
{
    switch (samples)
    {
        case 1:     return VK_SAMPLE_COUNT_1_BIT;
        case 2:     return VK_SAMPLE_COUNT_2_BIT;
        case 4:     return VK_SAMPLE_COUNT_4_BIT;
        case 8:     return VK_SAMPLE_COUNT_8_BIT;
        case 16:    return VK_SAMPLE_COUNT_16_BIT;
        case 32:    return VK_SAMPLE_COUNT_32_BIT;
        case 64:    return VK_SAMPLE_COUNT_64_BIT;
        default:    break;
    }
    throw std::invalid_argument(
        "failed to map multi-sampling of " + std::to_string(samples) +
        " sample(s) to <VkSampleCountFlagBits> Vulkan parameter"
    );
}

VkOffset3D ToVkOffset(const Offset3D& offset)
{
    return VkOffset3D{ offset.x, offset.y, offset.z };
}

VkExtent3D ToVkExtent(const Extent3D& extent)
{
    return VkExtent3D{ extent.width, extent.height, extent.depth };
}

Format Unmap(const VkFormat format)
{
    switch (format)
    {
        /* --- Red channel color formats --- */
        case VK_FORMAT_R8_UNORM:                    return Format::R8UNorm;
        case VK_FORMAT_R8_SNORM:                    return Format::R8SNorm;
        case VK_FORMAT_R8_UINT:                     return Format::R8UInt;
        case VK_FORMAT_R8_SINT:                     return Format::R8SInt;

        case VK_FORMAT_R16_UNORM:                   return Format::R16UNorm;
        case VK_FORMAT_R16_SNORM:                   return Format::R16SNorm;
        case VK_FORMAT_R16_UINT:                    return Format::R16UInt;
        case VK_FORMAT_R16_SINT:                    return Format::R16SInt;
        case VK_FORMAT_R16_SFLOAT:                  return Format::R16Float;

        case VK_FORMAT_R32_UINT:                    return Format::R32UInt;
        case VK_FORMAT_R32_SINT:                    return Format::R32SInt;
        case VK_FORMAT_R32_SFLOAT:                  return Format::R32Float;

        case VK_FORMAT_R64_SFLOAT:                  return Format::R64Float;

        /* --- RG color formats --- */
        case VK_FORMAT_R8G8_UNORM:                  return Format::RG8UNorm;
        case VK_FORMAT_R8G8_SNORM:                  return Format::RG8SNorm;
        case VK_FORMAT_R8G8_UINT:                   return Format::RG8UInt;
        case VK_FORMAT_R8G8_SINT:                   return Format::RG8SInt;

        case VK_FORMAT_R16G16_UNORM:                return Format::RG16UNorm;
        case VK_FORMAT_R16G16_SNORM:                return Format::RG16SNorm;
        case VK_FORMAT_R16G16_UINT:                 return Format::RG16UInt;
        case VK_FORMAT_R16G16_SINT:                 return Format::RG16SInt;
        case VK_FORMAT_R16G16_SFLOAT:               return Format::RG16Float;

        case VK_FORMAT_R32G32_UINT:                 return Format::RG32UInt;
        case VK_FORMAT_R32G32_SINT:                 return Format::RG32SInt;
        case VK_FORMAT_R32G32_SFLOAT:               return Format::RG32Float;

        case VK_FORMAT_R64G64_SFLOAT:               return Format::RG64Float;

        /* --- RGB color formats --- */
        case VK_FORMAT_R8G8B8_UNORM:                return Format::RGB8UNorm;
        case VK_FORMAT_R8G8B8_SRGB:                 return Format::RGB8UNorm_sRGB;
        case VK_FORMAT_R8G8B8_SNORM:                return Format::RGB8SNorm;
        case VK_FORMAT_R8G8B8_UINT:                 return Format::RGB8UInt;
        case VK_FORMAT_R8G8B8_SINT:                 return Format::RGB8SInt;

        case VK_FORMAT_R16G16B16_UNORM:             return Format::RGB16UNorm;
        case VK_FORMAT_R16G16B16_SNORM:             return Format::RGB16SNorm;
        case VK_FORMAT_R16G16B16_UINT:              return Format::RGB16UInt;
        case VK_FORMAT_R16G16B16_SINT:              return Format::RGB16SInt;
        case VK_FORMAT_R16G16B16_SFLOAT:            return Format::RGB16Float;

        case VK_FORMAT_R32G32B32_UINT:              return Format::RGB32UInt;
        case VK_FORMAT_R32G32B32_SINT:              return Format::RGB32SInt;
        case VK_FORMAT_R32G32B32_SFLOAT:            return Format::RGB32Float;

        case VK_FORMAT_R64G64B64_SFLOAT:            return Format::RGB64Float;

        /* --- RGBA color formats --- */
        case VK_FORMAT_R8G8B8A8_UNORM:              return Format::RGBA8UNorm;
        case VK_FORMAT_R8G8B8A8_SRGB:               return Format::RGBA8UNorm_sRGB;
        case VK_FORMAT_R8G8B8A8_SNORM:              return Format::RGBA8SNorm;
        case VK_FORMAT_R8G8B8A8_UINT:               return Format::RGBA8UInt;
        case VK_FORMAT_R8G8B8A8_SINT:               return Format::RGBA8SInt;

        case VK_FORMAT_R16G16B16A16_UNORM:          return Format::RGBA16UNorm;
        case VK_FORMAT_R16G16B16A16_SNORM:          return Format::RGBA16SNorm;
        case VK_FORMAT_R16G16B16A16_UINT:           return Format::RGBA16UInt;
        case VK_FORMAT_R16G16B16A16_SINT:           return Format::RGBA16SInt;
        case VK_FORMAT_R16G16B16A16_SFLOAT:         return Format::RGBA16Float;

        case VK_FORMAT_R32G32B32A32_UINT:           return Format::RGBA32UInt;
        case VK_FORMAT_R32G32B32A32_SINT:           return Format::RGBA32SInt;
        case VK_FORMAT_R32G32B32A32_SFLOAT:         return Format::RGBA32Float;

        case VK_FORMAT_R64G64B64A64_SFLOAT:         return Format::RGBA64Float;

        /* --- BGRA color formats --- */
        case VK_FORMAT_B8G8R8A8_UNORM:              return Format::BGRA8UNorm;
        case VK_FORMAT_B8G8R8A8_SRGB:               return Format::BGRA8UNorm_sRGB;
        case VK_FORMAT_B8G8R8A8_SNORM:              return Format::BGRA8SNorm;
        case VK_FORMAT_B8G8R8A8_UINT:               return Format::BGRA8UInt;
        case VK_FORMAT_B8G8R8A8_SINT:               return Format::BGRA8SInt;

        /* --- Packed formats --- */
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:    return Format::RGB10A2UNorm;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:     return Format::RGB10A2UInt;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:     return Format::RG11B10Float;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:      return Format::RGB9E5Float;

        /* --- Depth-stencil formats --- */
        case VK_FORMAT_D16_UNORM:                   return Format::D16UNorm;
        case VK_FORMAT_D32_SFLOAT:                  return Format::D32Float;
        case VK_FORMAT_D24_UNORM_S8_UINT:           return Format::D24UNormS8UInt;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:          return Format::D32FloatS8X24UInt;

        /* --- Block compression (BC) formats --- */
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:        return Format::BC1UNorm;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:         return Format::BC1UNorm_sRGB;
        case VK_FORMAT_BC2_UNORM_BLOCK:             return Format::BC2UNorm;
        case VK_FORMAT_BC2_SRGB_BLOCK:              return Format::BC2UNorm_sRGB;
        case VK_FORMAT_BC3_UNORM_BLOCK:             return Format::BC3UNorm;
        case VK_FORMAT_BC3_SRGB_BLOCK:              return Format::BC3UNorm_sRGB;
        case VK_FORMAT_BC4_UNORM_BLOCK:             return Format::BC4UNorm;
        case VK_FORMAT_BC4_SNORM_BLOCK:             return Format::BC4SNorm;
        case VK_FORMAT_BC5_UNORM_BLOCK:             return Format::BC5UNorm;
        case VK_FORMAT_BC5_SNORM_BLOCK:             return Format::BC5SNorm;

        default:                                    return Format::Undefined;
    }
}


/* ----- Convert functions ----- */

//TODO: let user decide to flip viewport
void Convert(VkViewport& dst, const Viewport& src)
{
    dst.x        = src.x;
    dst.y        = src.y + src.height;
    dst.width    = src.width;
    dst.height   = -src.height;
    dst.minDepth = src.minDepth;
    dst.maxDepth = src.maxDepth;
}

void Convert(VkRect2D& dst, const Scissor& src)
{
    dst.offset.x        = src.x;
    dst.offset.y        = src.y;
    dst.extent.width    = static_cast<std::uint32_t>(src.width);
    dst.extent.height   = static_cast<std::uint32_t>(src.height);
}

void Convert(VkRect2D& dst, const Viewport& src)
{
    dst.offset.x        = static_cast<std::int32_t>(src.x);
    dst.offset.y        = static_cast<std::int32_t>(src.y);
    dst.extent.width    = static_cast<std::uint32_t>(src.width);
    dst.extent.height   = static_cast<std::uint32_t>(src.height);
}


} // /namespace VKTypes

} // /namespace LLGL



// ================================================================================
