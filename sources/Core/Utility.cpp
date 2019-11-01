/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>
#include <LLGL/Buffer.h>
#include <LLGL/Texture.h>
#include <LLGL/Sampler.h>
#include <LLGL/Shader.h>
#include <LLGL/VertexFormat.h>
#include <cstring>
#include <cctype>


namespace LLGL
{


/* ----- TextureDescriptor utility functions ----- */

LLGL_EXPORT TextureDescriptor Texture1DDesc(Format format, std::uint32_t width, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture1D;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DDesc(Format format, std::uint32_t width, std::uint32_t height, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2D;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture3DDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture3D;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.extent.depth   = depth;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeDesc(Format format, std::uint32_t width, std::uint32_t height, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::TextureCube;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(Format format, std::uint32_t width, std::uint32_t arrayLayers, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture1DArray;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.arrayLayers    = arrayLayers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2DArray;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.arrayLayers    = arrayLayers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::TextureCubeArray;
        desc.bindFlags      = bindFlags;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.arrayLayers    = arrayLayers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2DMS;
        desc.bindFlags      = bindFlags;
        desc.miscFlags      = MiscFlags::FixedSamples;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.samples        = samples;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t arrayLayers, std::uint32_t samples, long bindFlags)
{
    TextureDescriptor desc;
    {
        desc.type           = TextureType::Texture2DMSArray;
        desc.bindFlags      = bindFlags;
        desc.miscFlags      = MiscFlags::FixedSamples;
        desc.format         = format;
        desc.extent.width   = width;
        desc.extent.height  = height;
        desc.arrayLayers    = arrayLayers;
        desc.samples        = samples;
    }
    return desc;
}

/* ----- BufferDescriptor utility functions ----- */

LLGL_EXPORT BufferDescriptor VertexBufferDesc(std::uint64_t size, const VertexFormat& vertexFormat, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size           = size;
        desc.bindFlags      = BindFlags::VertexBuffer;
        desc.cpuAccessFlags = cpuAccessFlags;
        desc.vertexAttribs  = vertexFormat.attributes;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor IndexBufferDesc(std::uint64_t size, const Format format, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size           = size;
        desc.format         = format;
        desc.bindFlags      = BindFlags::IndexBuffer;
        desc.cpuAccessFlags = cpuAccessFlags;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor ConstantBufferDesc(std::uint64_t size, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size           = size;
        desc.bindFlags      = BindFlags::ConstantBuffer;
        desc.cpuAccessFlags = cpuAccessFlags;
        desc.miscFlags      = MiscFlags::DynamicUsage;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor StorageBufferDesc(std::uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long cpuAccessFlags)
{
    BufferDescriptor desc;
    {
        desc.size                       = size;
        desc.stride                     = stride;
        desc.bindFlags                  = BindFlags::Storage;
        desc.cpuAccessFlags             = cpuAccessFlags;

        if (storageType == StorageBufferType::TypedBuffer       ||
            storageType == StorageBufferType::StructuredBuffer  ||
            storageType == StorageBufferType::ByteAddressBuffer)
        {
            desc.bindFlags |= BindFlags::Sampled;
        }
        if (storageType == StorageBufferType::AppendStructuredBuffer ||
            storageType == StorageBufferType::ConsumeStructuredBuffer)
        {
            desc.miscFlags |= MiscFlags::Append;
        }
    }
    return desc;
}

/* ----- ShaderDescriptor utility functions ----- */

LLGL_EXPORT ShaderDescriptor ShaderDescFromFile(const ShaderType type, const char* filename, const char* entryPoint, const char* profile, long flags)
{
    ShaderDescriptor desc;

    if (filename != nullptr)
    {
        if (auto fileExt = std::strrchr(filename, '.'))
        {
            /* Check if filename refers to a text-based source file */
            bool isTextFile = false;

            for (auto ext : { "hlsl", "fx", "glsl", "vert", "tesc", "tese", "geom", "frag", "comp", "metal" })
            {
                if (std::strcmp(fileExt + 1, ext) == 0)
                {
                    isTextFile = true;
                    break;
                }
            }

            /* Initialize descriptor */
            desc.type       = type;
            desc.source     = filename;
            desc.sourceSize = 0;
            desc.sourceType = (isTextFile ? ShaderSourceType::CodeFile : ShaderSourceType::BinaryFile);
            desc.entryPoint = entryPoint;
            desc.profile    = profile;
            desc.flags      = flags;
        }
    }

    return desc;
}

/* ----- ShaderProgramDescriptor utility functions ----- */

static void AssignShaderToDesc(ShaderProgramDescriptor& desc, Shader* shader)
{
    if (shader != nullptr)
    {
        /* Assign shader types their respective struct members */
        switch (shader->GetType())
        {
            case ShaderType::Undefined:
                break;
            case ShaderType::Vertex:
                desc.vertexShader = shader;
                break;
            case ShaderType::TessControl:
                desc.tessControlShader = shader;
                break;
            case ShaderType::TessEvaluation:
                desc.tessEvaluationShader = shader;
                break;
            case ShaderType::Geometry:
                desc.geometryShader = shader;
                break;
            case ShaderType::Fragment:
                desc.fragmentShader = shader;
                break;
            case ShaderType::Compute:
                desc.computeShader = shader;
                break;
        }
    }
}

LLGL_EXPORT ShaderProgramDescriptor ShaderProgramDesc(const std::initializer_list<Shader*>& shaders)
{
    ShaderProgramDescriptor desc;
    {
        for (auto shader : shaders)
            AssignShaderToDesc(desc, shader);
    }
    return desc;
}

LLGL_EXPORT ShaderProgramDescriptor ShaderProgramDesc(const std::vector<Shader*>& shaders)
{
    ShaderProgramDescriptor desc;
    {
        for (auto shader : shaders)
            AssignShaderToDesc(desc, shader);
    }
    return desc;
}

/* ----- PipelineLayoutDescriptor utility functions ----- */

LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const ShaderReflection& reflection)
{
    PipelineLayoutDescriptor desc;
    {
        desc.bindings.resize(reflection.resources.size());
        for (std::size_t i = 0; i < desc.bindings.size(); ++i)
            desc.bindings[i] = reflection.resources[i].binding;
    }
    return desc;
}

// Return name of ASCII character.
static std::string GetASCIIName(char c)
{
    if (c >= 0 && c <= 32)
    {
        static const char* g_names[] =
        {
            "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",     // 0x00 - 0x0F
            "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US",  // 0x10 - 0x1F
            "SP",                                                                                                       // 0x20
        };
        return ('<' + std::string(g_names[static_cast<std::size_t>(c)]) + '>');
    }
    else if (c == 127)
        return "<DEL>";
    return ('\'' + std::string(1, c) + '\'');
}

[[noreturn]]
static void ErrUnexpectedChar(const char* err, char c)
{
    throw std::invalid_argument(std::string(err) + ", but got " + GetASCIIName(c));
}

// Parse character or throw exception.
static void AcceptChar(const char*& s, char c, const char* err = nullptr)
{
    if (*s == c)
        ++s;
    else
    {
        if (err != nullptr)
            ErrUnexpectedChar(err, *s);
        else
        {
            std::string errMsg { "expected character " + GetASCIIName(c) };
            ErrUnexpectedChar(errMsg.c_str(), *s);
        }
    }
}

// Ignore all whitespace characters.
static void IgnoreWhiteSpaces(const char*& s)
{
    while (std::isspace(*s))
        ++s;
}

// Parse resource type identifier for layout signature, e.g. "texture".
static void ParseLayoutSignatureResourceType(const char*& s, ResourceType& type, long& bindFlags)
{
    struct ResourceTypeIdent
    {
        const char*     ident;
        ResourceType    type;
        long            bindFlags;
    };
    const ResourceTypeIdent g_resources[] =
    {
        { "cbuffer",   ResourceType::Buffer,  BindFlags::ConstantBuffer },
        { "buffer",    ResourceType::Buffer,  BindFlags::Sampled        },
        { "rwbuffer",  ResourceType::Buffer,  BindFlags::Storage        },
        { "texture",   ResourceType::Texture, BindFlags::Sampled        },
        { "rwtexture", ResourceType::Texture, BindFlags::Storage        },
        { "sampler",   ResourceType::Sampler, 0                         },
    };

    /* Parse identifier (find end of alphabetic characters) */
    IgnoreWhiteSpaces(s);

    auto token = s;
    while (std::isalpha(static_cast<unsigned char>(*s)))
        ++s;
    auto tokenLen = static_cast<std::size_t>(s - token);

    if (tokenLen > 0)
    {
        /* Determine which identifier is used */
        for (const auto& resource : g_resources)
        {
            if (std::strlen(resource.ident) == tokenLen && std::strncmp(token, resource.ident, tokenLen) == 0)
            {
                type        = resource.type;
                bindFlags   = resource.bindFlags;
                return;
            }
        }

        /* Identifier not found */
        throw std::invalid_argument("unknown resource type in layout signature: " + std::string(token, tokenLen));
    }

    ErrUnexpectedChar("expected resource type identifier", *s);
}

// Parse unsigned integral number.
static std::uint32_t ParseUInt32(const char*& s)
{
    IgnoreWhiteSpaces(s);

    if (*s >= '0' && *s <= '9')
    {
        std::uint32_t num = 0;
        while (*s >= '0' && *s <= '9')
        {
            num *= 10;
            num += (*s - '0');
            ++s;
        }

        return num;
    }

    ErrUnexpectedChar("expected numeric character", *s);
}

// Parse single shader stage flag identifier, e.g. "vert" or "frag".
static long ParseLayoutSignatureStageFlag(const char*& s)
{
    struct StageFlagIdent
    {
        long        bitmask;
        const char* ident;
    };
    const StageFlagIdent g_flags[] =
    {
        { StageFlags::VertexStage,          "vert" },
        { StageFlags::TessControlStage,     "tesc" },
        { StageFlags::TessEvaluationStage,  "tese" },
        { StageFlags::GeometryStage,        "geom" },
        { StageFlags::FragmentStage,        "frag" },
        { StageFlags::ComputeStage,         "comp" },
    };

    /* Parse identifier (find end of alphabetic characters) */
    IgnoreWhiteSpaces(s);

    auto token = s;
    while (std::isalpha(static_cast<unsigned char>(*s)))
        ++s;
    auto tokenLen = static_cast<std::size_t>(s - token);

    /* Determine which identifier is used */
    for (const auto& flag : g_flags)
    {
        if (std::strncmp(token, flag.ident, tokenLen) == 0)
            return flag.bitmask;
    }

    /* Identifier not found */
    throw std::invalid_argument("unknown shader stage in layout signature: " + std::string(token, tokenLen));
}

// Parse all shader stage flags, e.g. ":vert:frag".
static long ParseLayoutSignatureStageFlagsAll(const char*& s)
{
    long flags = 0;

    while (*s == ':')
    {
        ++s;
        IgnoreWhiteSpaces(s);
        flags |= ParseLayoutSignatureStageFlag(s);
        IgnoreWhiteSpaces(s);
    }

    return flags;
}

// Parse next layout signature binding point, e.g. "texture(1)"
static void ParseLayoutSignatureBindingPoint(PipelineLayoutDescriptor& desc, const char*& s)
{
    BindingDescriptor bindingDesc;

    /* Parse resource type and set stages to default */
    ParseLayoutSignatureResourceType(s, bindingDesc.type, bindingDesc.bindFlags);
    bindingDesc.stageFlags = StageFlags::AllStages;

    /* Parse binding points */
    IgnoreWhiteSpaces(s);
    AcceptChar(s, '(', "expected open bracket '(' after resource type");

    auto bindingIndex = desc.bindings.size();

    for (;;)
    {
        /* Parse optional name */
        IgnoreWhiteSpaces(s);
        bindingDesc.name.clear();

        if (std::isalpha(static_cast<unsigned char>(*s)) || *s == '_')
        {
            for (; std::isalnum(static_cast<unsigned char>(*s)) || *s == '_'; ++s)
                bindingDesc.name += *s;
            IgnoreWhiteSpaces(s);
            AcceptChar(s, '@');
        }

        /* Parse slot number */
        bindingDesc.slot = ParseUInt32(s);
        IgnoreWhiteSpaces(s);

        /* Parse optional array size */
        if (*s == '[')
        {
            ++s;
            bindingDesc.arraySize = ParseUInt32(s);
            IgnoreWhiteSpaces(s);
            AcceptChar(s, ']');
            IgnoreWhiteSpaces(s);
        }
        else
            bindingDesc.arraySize = 1;

        /* Add new binding point to output descriptor */
        desc.bindings.push_back(bindingDesc);

        if (*s == ',')
            ++s;
        else
            break;
    }

    AcceptChar(s, ')', "expected close bracket ')' after slot indices");
    IgnoreWhiteSpaces(s);

    /* Parse optional shader stage flags */
    if (*s == ':')
    {
        auto stageFlags = ParseLayoutSignatureStageFlagsAll(s);

        /* Update stage flags of all previously added binding descriptors */
        while (bindingIndex < desc.bindings.size())
            desc.bindings[bindingIndex++].stageFlags = stageFlags;
    }
}

// Parse layout signature, e.g. "texture(1), sampler(2)"
static void ParseLayoutSignature(PipelineLayoutDescriptor& desc, const char* s)
{
    while (*s != '\0')
    {
        /* Parse next binding point */
        ParseLayoutSignatureBindingPoint(desc, s);

        /* If there is no comma, the layout must end */
        if (*s == ',')
            ++s;
        else
        {
            IgnoreWhiteSpaces(s);
            if (*s != '\0')
                ErrUnexpectedChar("expected comma separator ',' after binding point", *s);
        }
    }
}

LLGL_EXPORT PipelineLayoutDescriptor PipelineLayoutDesc(const char* layoutSignature)
{
    PipelineLayoutDescriptor desc;
    {
        if (layoutSignature)
            ParseLayoutSignature(desc, layoutSignature);
        else
            throw std::invalid_argument("input parameter must not be null: layoutSignature");
    }
    return desc;
}

/* ----- RenderPassDescriptor utility functions ----- */

LLGL_EXPORT RenderPassDescriptor RenderPassDesc(const RenderTargetDescriptor& renderTargetDesc)
{
    RenderPassDescriptor desc;
    {
        for (const auto& attachment : renderTargetDesc.attachments)
        {
            /* First try to get format from texture */
            auto format = Format::Undefined;
            if (auto texture = attachment.texture)
                format = texture->GetDesc().format;

            switch (attachment.type)
            {
                case AttachmentType::Color:
                    desc.colorAttachments.push_back({ format });
                    break;

                case AttachmentType::Depth:
                    desc.depthAttachment    = { format != Format::Undefined ? format : Format::D32Float };
                    break;

                case AttachmentType::DepthStencil:
                    desc.depthAttachment    = { format != Format::Undefined ? format : Format::D24UNormS8UInt };
                    desc.stencilAttachment  = desc.depthAttachment;
                    break;

                case AttachmentType::Stencil:
                    desc.depthAttachment    = { format != Format::Undefined ? format : Format::D24UNormS8UInt };
                    break;
            }
        }
    }
    return desc;
}


} // /namespace LLGL

#endif



// ================================================================================
