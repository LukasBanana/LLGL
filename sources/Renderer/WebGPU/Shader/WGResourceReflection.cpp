/*
 * WGResourceReflection.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGResourceReflection.h"
#include "WGResourceReflectionTable.h"
#include "WGSLScanner.h"
#include "../WGCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/BasicParser.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/StringUtils.h"
#include <LLGL/Container/Strings.h>
#include <functional>
#include <initializer_list>


namespace LLGL
{


#define LLGL_ERRORF(REPORT, ...)            \
    do                                      \
    {                                       \
        if (Report* _Report_ = (REPORT))    \
            _Report_->Errorf(__VA_ARGS__);  \
    }                                       \
    while (false)

class WGSLResourceParser;

using WGSLParseFunction = std::function<bool(WGSLResourceParser&)>;
using WGSLAcceptTokenFunction = const std::function<bool(const WGSLToken& tok)>;


/*
 * WGSLResourceParser class
 */

class WGSLResourceParser : public BasicParser<WGSLToken>
{

    public:

        WGSLResourceParser() = default;
        WGSLResourceParser(ArrayView<WGSLToken> tokens);

    public:

        void ErrorUnexpectedToken(Report* outReport, StringView expectedSpell = {}, int offset = 0);

        bool Match(WGSLTokenType type, int offset = 0) const;
        bool Match(WGSLTokenType type, StringView spell, int offset = 0) const;

        const WGSLToken* AcceptIf(WGSLTokenType type, Report* outReport = nullptr);
        const WGSLToken* AcceptIf(WGSLTokenType type, StringView spell, Report* outReport = nullptr);
        const WGSLToken* AcceptIf(const WGSLAcceptTokenFunction& predicate, Report* outReport = nullptr);

        int AcceptUntil(const std::function<bool(const WGSLToken& tok)>& predicate);

    public:

        WGResourceReflectionTable resourceTable;

};

WGSLResourceParser::WGSLResourceParser(ArrayView<WGSLToken> tokens) :
    BasicParser{ tokens }
{
}

void WGSLResourceParser::ErrorUnexpectedToken(Report* outReport, StringView expectedSpell, int offset)
{
    if (outReport != nullptr)
    {
        StringView tok = Token(offset).spell;
        if (expectedSpell.empty())
        {
            outReport->Errorf(
                "unexpected token '%.*s'\n",
                static_cast<int>(tok.size()), tok.data()
            );
        }
        else
        {
            outReport->Errorf(
                "unexpected token '%.*s'; expected '%.*s'\n",
                static_cast<int>(tok.size()), tok.data(),
                static_cast<int>(expectedSpell.size()), expectedSpell.data()
            );
        }
    }
}

bool WGSLResourceParser::Match(WGSLTokenType type, int offset) const
{
    return (Token(offset).type == type);
}

bool WGSLResourceParser::Match(WGSLTokenType type, StringView spell, int offset) const
{
    const WGSLToken& tok = Token(offset);
    return (tok.type == type && tok.spell == spell);
}

const WGSLToken* WGSLResourceParser::AcceptIf(WGSLTokenType type, Report* outReport)
{
    if (Match(type))
    {
        const WGSLToken& tok = Accept();
        return &tok;
    }
    ErrorUnexpectedToken(outReport);
    return nullptr;
}

const WGSLToken* WGSLResourceParser::AcceptIf(WGSLTokenType type, StringView spell, Report* outReport)
{
    if (Match(type, spell))
    {
        const WGSLToken& tok = Accept();
        return &tok;
    }
    ErrorUnexpectedToken(outReport, spell);
    return nullptr;
}

const WGSLToken* WGSLResourceParser::AcceptIf(const WGSLAcceptTokenFunction& predicate, Report* outReport)
{
    const WGSLToken& tok = Token();
    if (predicate(tok))
        return &tok;
    ErrorUnexpectedToken(outReport);
    return nullptr;
}

int WGSLResourceParser::AcceptUntil(const std::function<bool(const WGSLToken& tok)>& predicate)
{
    int numAcceptedTokens = 0;
    while (Feed() && !predicate(Token()))
        Accept();
    return numAcceptedTokens;
}


/*
 * Internal WGSL parsing functions
 */

static bool IsWGSLTokenLCurly(const WGSLToken& tok)
{
    return (tok.type == WGSLTokenType::Punctuation && tok.spell == "{");
}

static bool IsWGSLTokenRCurly(const WGSLToken& tok)
{
    return (tok.type == WGSLTokenType::Punctuation && tok.spell == "}");
}

static bool IsWGSLTokenLorRCurly(const WGSLToken& tok)
{
    return (tok.type == WGSLTokenType::Punctuation && (tok.spell == "{" || tok.spell == "}"));
}

static bool IsWGSLTokenEndOfStatement(const WGSLToken& tok)
{
    return (tok.type == WGSLTokenType::Punctuation && tok.spell == ";");
}

static bool ParseCodeBlock(WGSLResourceParser& parser, Report* outReport, const WGSLParseFunction& parseBody = nullptr)
{
    if (parser.AcceptIf(WGSLTokenType::Punctuation, "{", outReport) == nullptr)
        return false;

    while (parser.Feed())
    {
        if (parser.AcceptIf(WGSLTokenType::Punctuation, "}") != nullptr)
        {
            /* Close code block */
            return true;
        }
        else if (parser.Match(WGSLTokenType::Punctuation, "{"))
        {
            /* Open nested code block */
            if (!ParseCodeBlock(parser, outReport, parseBody))
                return false;
        }
        else if (parseBody)
        {
            /* Parse code block body */
            parseBody(parser);
        }
        else
        {
            /* Fast foward to the next code block token */
            parser.AcceptUntil(IsWGSLTokenLorRCurly);
        }
    }

    LLGL_ERRORF(outReport, "unbalanced code block in WGSL source { ... }\n");
    return false;
}

static bool SkipUntilEndOfCodeBlock(WGSLResourceParser& parser, Report* outReport)
{
    parser.AcceptUntil(IsWGSLTokenLCurly);
    return ParseCodeBlock(parser, outReport);
}

static bool SkipUntilEndOfStatement(WGSLResourceParser& parser, Report* outReport)
{
    parser.AcceptUntil(IsWGSLTokenEndOfStatement);
    return parser.AcceptIf(WGSLTokenType::Punctuation, ";", outReport);
}

static bool ParseStructDecl(WGSLResourceParser& parser, Report* outReport)
{
    return (parser.AcceptIf(WGSLTokenType::Identifier, "struct", outReport) != nullptr && SkipUntilEndOfCodeBlock(parser, outReport));
}

static bool ParseFunctionDecl(WGSLResourceParser& parser, Report* outReport)
{
    return (parser.AcceptIf(WGSLTokenType::Identifier, "fn", outReport) != nullptr && SkipUntilEndOfCodeBlock(parser, outReport));
}

static bool ParseIntAttribute(WGSLResourceParser& parser, Report* outReport, StringView attribName, std::uint64_t& outValue)
{
    parser.AcceptIf(WGSLTokenType::Punctuation, "@");

    if (parser.AcceptIf(WGSLTokenType::Identifier, attribName, outReport) == nullptr)
        return false;
    if (parser.AcceptIf(WGSLTokenType::Punctuation, "(", outReport) == nullptr)
        return false;

    const WGSLToken* tokIntLiteral = parser.AcceptIf(WGSLTokenType::IntLiteral, outReport);
    if (tokIntLiteral == nullptr)
        return false;

    if (!parser.AcceptIf(WGSLTokenType::Punctuation, ")", outReport))
        return false;

    if (!ScanWGSLIntLiteral(tokIntLiteral->spell, outValue, nullptr, outReport))
    {
        LLGL_ERRORF(outReport, "failed to parse @%.*s() attribute", static_cast<int>(attribName.size()), attribName.data());
        return false;
    }

    return true;
}

static bool ParseTemplateWith1OptionalParam(WGSLResourceParser& parser, Report* outReport, StringView typeName, const WGSLToken*& outTokTemplateArg0)
{
    if (parser.AcceptIf(WGSLTokenType::Identifier, typeName, outReport) == nullptr)
        return false;

    if (parser.AcceptIf(WGSLTokenType::Punctuation, "<") != nullptr)
    {
        outTokTemplateArg0 = &(parser.Accept());
        if (parser.AcceptIf(WGSLTokenType::Punctuation, ">", outReport) == nullptr)
            return false;
    }

    return true;
}

static bool ParseVariableDecl(WGSLResourceParser& parser, Report* outReport, bool& outIsUniformDecl)
{
    const WGSLToken* tokVarType = nullptr;
    if (!ParseTemplateWith1OptionalParam(parser, outReport, "var", tokVarType))
        return false;

    outIsUniformDecl = (tokVarType != nullptr && tokVarType->type == WGSLTokenType::Identifier && tokVarType->spell == "uniform");

    return true;
}

// See https://www.w3.org/TR/WGSL/#sampled-texture-type
static WGPUTextureSampleType MapTypenameToSampleType(const WGResourceReflectionTable& resourceTable, StringView ident)
{
    /* Try to map to basic types */
    if (ident == "f32") { return WGPUTextureSampleType_Float; }
    if (ident == "i32") { return WGPUTextureSampleType_Sint;  }
    if (ident == "u32") { return WGPUTextureSampleType_Uint;  }

    /* Try to find identifier in type aliases map */
    auto typeAliasIt = resourceTable.typeAliases.find(std::string(ident.begin(), ident.end()));
    if (typeAliasIt != resourceTable.typeAliases.end() &&
        typeAliasIt->second < resourceTable.resourceTypes.size())
    {
        const WGResourceReflection& aliasedResourceType = resourceTable.resourceTypes[typeAliasIt->second];
        return aliasedResourceType.textureSampleType;
    }

    return WGPUTextureSampleType_Undefined;
}

static bool ParseTextureTypeArgs(WGSLResourceParser& parser, Report* outReport, WGResourceReflection& outResource)
{
    /* Parse texture subtype `< SUBTYPE >` */
    if (parser.AcceptIf(WGSLTokenType::Punctuation, "<", outReport) == nullptr)
        return false;

    const WGSLToken* tokSubtype = parser.AcceptIf(WGSLTokenType::Identifier, outReport);
    if (tokSubtype == nullptr)
        return false;

    if (parser.AcceptIf(WGSLTokenType::Punctuation, ">", outReport) == nullptr)
        return false;

    /* Map typename to sample type */
    outResource.textureSampleType = MapTypenameToSampleType(parser.resourceTable, tokSubtype->spell);
    if (outResource.textureSampleType == WGPUTextureSampleType_Undefined)
    {
        LLGL_ERRORF(
            outReport, "failed to map typename '%.*s' to texture sample type\n",
            static_cast<int>(tokSubtype->spell.size()), tokSubtype->spell.data()
        );
        return false;
    }

    return true;
}

// See https://www.w3.org/TR/WGSL/#texel-formats
static WGPUTextureFormat MapIdentToTextureFormat(StringView ident)
{
    for (std::pair<const char*, WGPUTextureFormat> it :
        std::initializer_list<std::pair<const char*, WGPUTextureFormat>>
        {
            { "rgba8unorm"   , WGPUTextureFormat_RGBA8Unorm    },
            { "rgba8snorm"   , WGPUTextureFormat_RGBA8Snorm    },
            { "rgba8uint"    , WGPUTextureFormat_RGBA8Uint     },
            { "rgba8sint"    , WGPUTextureFormat_RGBA8Sint     },
            { "rgba16unorm"  , WGPUTextureFormat_RGBA16Unorm   },
            { "rgba16snorm"  , WGPUTextureFormat_RGBA16Snorm   },
            { "rgba16uint"   , WGPUTextureFormat_RGBA16Uint    },
            { "rgba16sint"   , WGPUTextureFormat_RGBA16Sint    },
            { "rgba16float"  , WGPUTextureFormat_RGBA16Float   },
            { "rg8unorm"     , WGPUTextureFormat_RG8Unorm      },
            { "rg8snorm"     , WGPUTextureFormat_RG8Snorm      },
            { "rg8uint"      , WGPUTextureFormat_RG8Uint       },
            { "rg8sint"      , WGPUTextureFormat_RG8Sint       },
            { "rg16unorm"    , WGPUTextureFormat_RG16Unorm     },
            { "rg16snorm"    , WGPUTextureFormat_RG16Snorm     },
            { "rg16uint"     , WGPUTextureFormat_RG16Uint      },
            { "rg16sint"     , WGPUTextureFormat_RG16Sint      },
            { "rg16float"    , WGPUTextureFormat_RG16Float     },
            { "r32uint"      , WGPUTextureFormat_R32Uint       },
            { "r32sint"      , WGPUTextureFormat_R32Sint       },
            { "r32float"     , WGPUTextureFormat_R32Float      },
            { "rg32uint"     , WGPUTextureFormat_RG32Uint      },
            { "rg32sint"     , WGPUTextureFormat_RG32Sint      },
            { "rg32float"    , WGPUTextureFormat_RG32Float     },
            { "rgba32uint"   , WGPUTextureFormat_RGBA32Uint    },
            { "rgba32sint"   , WGPUTextureFormat_RGBA32Sint    },
            { "rgba32float"  , WGPUTextureFormat_RGBA32Float   },
            { "bgra8unorm"   , WGPUTextureFormat_BGRA8Unorm    },
            { "r8unorm"      , WGPUTextureFormat_R8Unorm       },
            { "r8snorm"      , WGPUTextureFormat_R8Snorm       },
            { "r8uint"       , WGPUTextureFormat_R8Uint        },
            { "r8sint"       , WGPUTextureFormat_R8Sint        },
            { "r16unorm"     , WGPUTextureFormat_R16Unorm      },
            { "r16snorm"     , WGPUTextureFormat_R16Snorm      },
            { "r16uint"      , WGPUTextureFormat_R16Uint       },
            { "r16sint"      , WGPUTextureFormat_R16Sint       },
            { "r16float"     , WGPUTextureFormat_R16Float      },
            { "rgb10a2unorm" , WGPUTextureFormat_RGB10A2Unorm  },
            { "rgb10a2uint"  , WGPUTextureFormat_RGB10A2Uint   },
            { "rg11b10ufloat", WGPUTextureFormat_RG11B10Ufloat },
        })
    {
        if (ident == it.first)
            return it.second;
    }
    return WGPUTextureFormat_Undefined;
}

// See https://www.w3.org/TR/WGSL/#memory-access-mode
static WGPUStorageTextureAccess MapIdentToStorageTextureAccess(StringView ident)
{
    if (ident == "write"     ) { return WGPUStorageTextureAccess_WriteOnly; }
    if (ident == "read"      ) { return WGPUStorageTextureAccess_ReadOnly ; }
    if (ident == "read_write") { return WGPUStorageTextureAccess_ReadWrite; }
    return WGPUStorageTextureAccess_Undefined;
}

static bool ParseStorageTextureTypeArgs(WGSLResourceParser& parser, Report* outReport, WGResourceReflection& outResource)
{
    /* Parse storagE texture format and access `< FORMAT , ACCESS >` */
    if (parser.AcceptIf(WGSLTokenType::Punctuation, "<", outReport) == nullptr)
        return false;

    const WGSLToken* tokFormat = parser.AcceptIf(WGSLTokenType::Identifier, outReport);
    if (tokFormat == nullptr)
        return false;

    if (parser.AcceptIf(WGSLTokenType::Punctuation, ",", outReport) == nullptr)
        return false;

    const WGSLToken* tokAccess = parser.AcceptIf(WGSLTokenType::Identifier, outReport);
    if (tokAccess == nullptr)
        return false;

    if (parser.AcceptIf(WGSLTokenType::Punctuation, ">", outReport) == nullptr)
        return false;

    /* Map identifiers to eumeration values */
    outResource.storageTextureFormat = MapIdentToTextureFormat(tokFormat->spell);
    if (outResource.storageTextureFormat == WGPUTextureFormat_Undefined)
    {
        LLGL_ERRORF(
            outReport, "failed to map identifier '%.*s' to storage texture format\n",
            static_cast<int>(tokFormat->spell.size()), tokFormat->spell.data()
        );
        return false;
    }

    outResource.storageTextureAccess = MapIdentToStorageTextureAccess(tokAccess->spell);
    if (outResource.storageTextureAccess == WGPUStorageTextureAccess_Undefined)
    {
        LLGL_ERRORF(
            outReport, "failed to map identifier '%.*s' to storage texture access mode\n",
            static_cast<int>(tokAccess->spell.size()), tokAccess->spell.data()
        );
        return false;
    }

    return false;
}

static bool ParseResourceType(WGSLResourceParser& parser, Report* outReport, WGResourceReflection& outResource)
{
    const WGSLToken* tokIdent = parser.AcceptIf(WGSLTokenType::Identifier, outReport);
    if (tokIdent == nullptr)
        return false;

    const StringView ident = tokIdent->spell;

    /* --- Sampler states --- */

    if (ident == "sampler")
    {
        outResource.samplerBindingType = WGPUSamplerBindingType_Filtering;
        return true;
    }
    else if (ident == "sampler_comparison")
    {
        outResource.samplerBindingType = WGPUSamplerBindingType_Comparison;
        return true;
    }

    /* --- Sampled textures --- */

    else if (ident == "texture_1d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_1D;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_1d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_1D;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_2d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_2D;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_2d_array")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_2DArray;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_3d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_3D;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_cube")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_Cube;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_cube_array")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_CubeArray;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }

    /* --- Multisampled textures --- */

    else if (ident == "texture_multisampled_2d")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_2D;
        outResource.multisampled            = WGPU_TRUE;
        return ParseTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_depth_multisampled_2d")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_2D;
        outResource.textureSampleType       = WGPUTextureSampleType_Depth;
        outResource.multisampled            = WGPU_TRUE;
        return true;
    }

    /* --- External textures --- */

    else if (ident == "texture_external")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_2D;
        outResource.textureSampleType       = WGPUTextureSampleType_Float; //???
        return true;
    }

    /* --- Storage textures --- */

    else if (ident == "texture_storage_1d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_1D;
        return ParseStorageTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_storage_2d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_2D;
        return ParseStorageTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_storage_2d_array")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_2DArray;
        return ParseStorageTextureTypeArgs(parser, outReport, outResource);
    }
    else if (ident == "texture_storage_3d")
    {
        outResource.textureViewDimension = WGPUTextureViewDimension_3D;
        return ParseStorageTextureTypeArgs(parser, outReport, outResource);
    }

    /* --- Depth textures --- */

    else if (ident == "texture_depth_2d")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_2D;
        outResource.textureSampleType       = WGPUTextureSampleType_Depth;
        return true;
    }
    else if (ident == "texture_depth_2d_array")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_2DArray;
        outResource.textureSampleType       = WGPUTextureSampleType_Depth;
        return true;
    }
    else if (ident == "texture_depth_cube")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_Cube;
        outResource.textureSampleType       = WGPUTextureSampleType_Depth;
        return true;
    }
    else if (ident == "texture_depth_cube_array")
    {
        outResource.textureViewDimension    = WGPUTextureViewDimension_CubeArray;
        outResource.textureSampleType       = WGPUTextureSampleType_Depth;
        return true;
    }

    /* --- Miscellaneous type specifiers --- */

    else
    {
        parser.AcceptUntil(IsWGSLTokenEndOfStatement); //TODO
        return true;
    }

    return false;
}

static bool ParseBindingDecl(WGSLResourceParser& parser, Report* outReport)
{
    WGResourceReflection resourceType;

    /* Parse group and binding attributes */
    std::uint64_t groupValue = 0;
    if (!ParseIntAttribute(parser, outReport, "group", groupValue))
        return false;
    resourceType.groupIndex = static_cast<std::uint32_t>(groupValue);

    std::uint64_t bindingValue = 0;
    if (!ParseIntAttribute(parser, outReport, "binding", bindingValue))
        return false;
    resourceType.bindingIndex = static_cast<std::uint32_t>(bindingValue);

    /* Parse uniform variable declaration */
    bool isUniformDecl = false;
    if (!ParseVariableDecl(parser, outReport, isUniformDecl))
        return false;

    const WGSLToken* tokName = parser.AcceptIf(WGSLTokenType::Identifier, outReport);
    if (tokName == nullptr)
        return false;

    /* Parse type specifier */
    if (!parser.AcceptIf(WGSLTokenType::Punctuation, ":", outReport))
        return false;

    if (!ParseResourceType(parser, outReport, resourceType))
        return false;
    const std::size_t resourceTypeIndex = parser.resourceTable.RegisterResourceType(std::move(resourceType));

    /* Insert new entry into resource table */
    std::string resourceName{ tokName->spell.begin(), tokName->spell.end() };
    auto resourceIt = parser.resourceTable.resources.find(resourceName);
    if (resourceIt != parser.resourceTable.resources.end())
    {
        LLGL_ERRORF(outReport, "duplicate delcaration of resource '%s'\n", resourceName.c_str());
        return false;
    }

    parser.resourceTable.resources.insert(
        resourceIt,
        std::pair<std::string, std::size_t>{ std::move(resourceName), resourceTypeIndex }
    );

    return parser.AcceptIf(WGSLTokenType::Punctuation, ";", outReport);
}

static bool ParseGlobalStatement(WGSLResourceParser& parser, Report* outReport)
{
    /* Function declaration */
    if (parser.AcceptIf(WGSLTokenType::Punctuation, "@"))
    {
        if (parser.AcceptIf(WGSLTokenType::Identifier, "vertex") ||
            parser.AcceptIf(WGSLTokenType::Identifier, "fragment"))
        {
            return ParseFunctionDecl(parser, outReport);
        }
        if (parser.Match(WGSLTokenType::Identifier, "group"))
        {
            return ParseBindingDecl(parser, outReport);
        }
    }

    /* Struct declaration */
    if (parser.Match(WGSLTokenType::Identifier, "struct"))
        return ParseStructDecl(parser, outReport);

    /* Common function (non-entry-point) */
    if (parser.Match(WGSLTokenType::Identifier, "fn"))
        return ParseFunctionDecl(parser, outReport);

    /* Empty statement */
    if (parser.AcceptIf(WGSLTokenType::Punctuation, ";"))
        return true;

    parser.ErrorUnexpectedToken(outReport);
    return false;
}

#undef LLGL_ERRORF


/*
 * WGResourceReflection class
 */

bool ReflectWGSLShaderSource(WGResourceReflectionTable& outReflectionTable, StringView sourceWGSL, Report* outReport)
{
    /* Scan tokens from WGSL source */
    DynamicVector<WGSLToken> tokens;
    ScanWGSLTokens(sourceWGSL, tokens);
    if (tokens.empty())
        return false;

    /* Parse tokens source tree */
    WGSLResourceParser parser{ tokens };
    while (parser.Feed())
    {
        if (!ParseGlobalStatement(parser, outReport))
            return false;
    }

    /* Take ownership of parsed resources */
    outReflectionTable = std::move(parser.resourceTable);

    return true;
}


} // /namespace LLGL



// ================================================================================
