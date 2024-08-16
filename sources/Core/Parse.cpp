/*
 * Parse.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/Strings.h>
#include <LLGL/Report.h>
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include "Exception.h"
#include "StringUtils.h"


namespace LLGL
{


static bool IsCharAlpha(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static bool IsCharNumeric(char c)
{
    return (c >= '0' && c <= '9');
}

static bool IsCharNumericHex(char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static bool IsCharIdentifier(char c)
{
    return (IsCharAlpha(c) || IsCharNumeric(c) || c == '_');
}

static bool IsCharWhitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\v' || c == '\n' || c == '\r');
}

#if 0 //UNUSED
// Return name of ASCII character.
static const char* GetASCIIName(char c)
{
    if (c >= 0 && c <= 32)
    {
        static const char* asciiTable[] =
        {
            "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",     // 0x00 - 0x0F
            "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US",  // 0x10 - 0x1F
            "SP",                                                                                                       // 0x20
        };
        return asciiTable[static_cast<std::size_t>(c)];
    }
    else if (c == 127)
    {
        /* Return ASCII name for special character */
        return "DEL";
    }
    else
    {
        /* Return input character as NUL-terminated string */
        static thread_local char buf[2] = { '\0', '\0' };
        buf[0] = c;
        return buf;
    }
}
#endif

static void ScanTokens(const char* start, const char* end, ParseContext::TokenArrayType& outTokens)
{
    /* Stores the current token */
    auto NextToken = [&start, &outTokens](const char* s)
    {
        outTokens.push_back(StringView{ start, static_cast<std::size_t>(s - start) });
    };

    for (const char* s = start; s != end; start = s)
    {
        if (IsCharIdentifier(*s))
        {
            /* Accept alpha-numeric token */
            while (s != end && IsCharIdentifier(*s))
                ++s;
            NextToken(s);
        }
        else if (IsCharWhitespace(*s))
        {
            /* Ignore whitespaces */
            while (s != end && IsCharWhitespace(*s))
                ++s;
        }
        else
        {
            /* Accept as single character token such as punctuation (e.g. '{' or '}') */
            ++s;
            NextToken(s);
        }
    }
}

static void ReserveAndScanTokens(ParseContext::StringType& source, ParseContext::TokenArrayType& outTokens)
{
    /* Reserve token array with average token length */
    constexpr std::size_t averageTokenLength = 8;
    outTokens.reserve(source.size() / averageTokenLength);
    ScanTokens(source.begin(), source.end(), outTokens);
}


/*
 * Parser class
 */

class Parser;

static bool ReturnWithParseError(Parser& parser, const char* format);
static bool ReturnWithParseError(Parser& parser, const char* format, const StringView& tok);

class Parser
{

    public:

        Parser() = default;

        Parser(const ArrayView<StringView>& tokens);

    public:

        // Resets the internal token iterator.
        void Reset();

        // Returns the current token with an optional offset.
        StringView Token(int offset = 0) const;

        // Returns true if the current token (with optional offset) matches the specified string.
        bool Match(const StringView& match, int offset = 0) const;

        // Returns true if the current token (with optional offset) matches an identifier.
        bool MatchIdent(int offset = 0) const;

        // Returns the base of the current token (with optional offset) if it matches a number. Otherwise, returns 0.
        int MatchNumeric(int offset = 0) const;

        // Accepts and returns the current token, then moves to the next token.
        StringView Accept();

        // Accepts the current token if it matches the specified string.
        bool Accept(const StringView& match);

        // Returns true if there are further tokens to parse.
        bool Feed() const;

        // Returns a new parser with a sub region until the specified matching end token.
        bool Fork(const StringView& matchEnd, Parser& outForkedParser);

    public:

        // Error report.
        Report report;

    private:

        ArrayView<StringView>   tokens_;
        std::size_t             iter_   = 0;

};

Parser::Parser(const ArrayView<StringView>& tokens) :
    tokens_ { tokens }
{
}

void Parser::Reset()
{
    iter_ = 0;
}

StringView Parser::Token(int offset) const
{
    if (offset < 0)
    {
        unsigned uOffset = static_cast<unsigned>(-offset);
        if (uOffset < iter_ && iter_ - uOffset < tokens_.size())
            return tokens_[iter_ - uOffset];
    }
    else if (offset > 0)
    {
        unsigned uOffset = static_cast<unsigned>(offset);
        if (iter_ + uOffset < tokens_.size())
            return tokens_[iter_ + uOffset];
    }
    else if (iter_ < tokens_.size())
    {
        /* Return current token */
        return tokens_[iter_];
    }
    return {};
}

bool Parser::Match(const StringView& match, int offset) const
{
    return (Token(offset) == match);
}

bool Parser::MatchIdent(int offset) const
{
    const StringView tok = Token(offset);
    if (tok.empty())
        return false;
    if (IsCharNumeric(tok.front()))
        return false;
    for (char c : tok)
    {
        if (!IsCharIdentifier(c))
            return false;
    }
    return true;
}

int Parser::MatchNumeric(int offset) const
{
    const StringView tok = Token(offset);
    if (tok.empty())
        return 0;
    if (tok.size() >= 3 && tok.compare(0, 2, "0x") == 0)
    {
        /* Match hexadecimal number */
        for (char c : tok.substr(2))
        {
            if (!IsCharNumericHex(c))
                return 0;
        }
        return 16;
    }
    else
    {
        /* Match decimal number */
        for (char c : tok)
        {
            if (!IsCharNumeric(c))
                return 0;
        }
        return 10;
    }
}

StringView Parser::Accept()
{
    if (iter_ < tokens_.size())
    {
        StringView s = Token();
        ++iter_;
        return s;
    }
    return {};
}

bool Parser::Accept(const StringView& match)
{
    if (Match(match))
    {
        ++iter_;
        return true;
    }
    return false;
}

bool Parser::Feed() const
{
    return (iter_ < tokens_.size());
}

bool Parser::Fork(const StringView& matchEnd, Parser& outForkedParser)
{
    /* Find matching end token */
    for (std::size_t start = iter_; Feed(); Accept())
    {
        if (Match(matchEnd))
        {
            outForkedParser = Parser{ ArrayView<StringView>{ (tokens_.data() + start), (iter_ - start) } };
            Accept();
            return true;
        }
    }

    /* Could not find matching end */
    return ReturnWithParseError(*this, "could not find matching end of sub section: %s", matchEnd);
}


/*
 * ParseContext class
 */

ParseContext::ParseContext(const StringView& source) :
    source_ { source.begin(), source.end() }
{
    ReserveAndScanTokens(source_, tokens_);
}

ParseContext::ParseContext(UTF8String&& source) :
    data_   { std::move(source)          },
    source_ { data_.begin(), data_.end() }
{
    ReserveAndScanTokens(source_, tokens_);
}

template <typename T>
struct DictionaryEntry
{
    const char* ident;
    T           value;
};

template <typename T>
using Dictionary = ArrayView<DictionaryEntry<T>>;

template <typename T>
static bool ParseValueFromDictionary(Parser& parser, const Dictionary<T>& dict, T& outValue, const char* valueName)
{
    const StringView tok = parser.Accept();
    if (tok.empty())
    {
        parser.report.Errorf("expected %s", valueName);
        return false;
    }

    for (const auto& entry : dict)
    {
        if (tok == entry.ident)
        {
            outValue = entry.value;
            return true;
        }
    }

    std::string tokStr{ tok.begin(), tok.end() };
    parser.report.Errorf("unknown %s: %s", valueName, tokStr.c_str());
    return false;
}

static bool ParseSamplerDesc(Parser& parser, SamplerDescriptor& outDesc);

static bool ReturnWithParseError(Parser& parser, const char* format)
{
    const StringView prevTok = parser.Token(-1);
    if (!prevTok.empty())
    {
        std::string prevTokStr{ prevTok.begin(), prevTok.end() };
        parser.report.Errorf("%s; last token = '%s'", format, prevTokStr.c_str());
    }
    else
        parser.report.Errorf(format);
    return false;
}

static bool ReturnWithParseError(Parser& parser, const char* format, const StringView& tok)
{
    std::string tokStr{ tok.begin(), tok.end() };
    parser.report.Errorf(format, tokStr.c_str());
    return false;
}

static bool ParseLayoutSignatureResourceType(Parser& parser, ResourceType& outType, long& outBindFlags)
{
    /* Parse resource type identifier */
    const StringView tok = parser.Token();
    if (tok.empty())
        return ReturnWithParseError(parser, "expected resource type identifier");

    struct ResourceTypeIdent
    {
        const char*     ident;
        ResourceType    type;
        long            bindFlags;
    };

    constexpr ResourceTypeIdent acceptedResources[] =
    {
        { "cbuffer",   ResourceType::Buffer,  BindFlags::ConstantBuffer },
        { "buffer",    ResourceType::Buffer,  BindFlags::Sampled        },
        { "rwbuffer",  ResourceType::Buffer,  BindFlags::Storage        },
        { "texture",   ResourceType::Texture, BindFlags::Sampled        },
        { "rwtexture", ResourceType::Texture, BindFlags::Storage        },
        { "sampler",   ResourceType::Sampler, 0                         },
    };

    for (const ResourceTypeIdent& resource : acceptedResources)
    {
        if (parser.Accept(resource.ident))
        {
            /* Accept token and return resource type with binding flags */
            outType         = resource.type;
            outBindFlags    = resource.bindFlags;
            return true;
        }
    }

    /* Unknown resource type identifier */
    return ReturnWithParseError(parser, "unknown resource type in layout signature: %s", tok);
}

// Parse single shader stage flag identifier, e.g. "vert" or "frag".
static long ParseLayoutSignatureStageFlag(Parser& parser, long& bitmask)
{
    struct StageFlagIdent
    {
        long        bitmask;
        const char* ident;
    };
    constexpr StageFlagIdent acceptedFlags[] =
    {
        { StageFlags::VertexStage,          "vert" },
        { StageFlags::TessControlStage,     "tesc" },
        { StageFlags::TessEvaluationStage,  "tese" },
        { StageFlags::GeometryStage,        "geom" },
        { StageFlags::FragmentStage,        "frag" },
        { StageFlags::ComputeStage,         "comp" },
    };

    /* Parse identifier (find end of alphabetic characters) */
    const StringView tok = parser.Token();
    if (tok.empty())
        ReturnWithParseError(parser, "expected stage flag identifier");

    /* Determine which identifier is used */
    for (const StageFlagIdent& flag : acceptedFlags)
    {
        if (parser.Accept(flag.ident))
        {
            bitmask |= flag.bitmask;
            return true;
        }
    }

    /* Identifier not found */
    return ReturnWithParseError(parser, "unknown shader stage in layout signature: %s", tok);
}

// Parse all shader stage flags, e.g. ":vert:frag".
static bool ParseLayoutSignatureStageFlagsAll(Parser& parser, long& outStageFlags)
{
    while (parser.Accept(":"))
    {
        if (!ParseLayoutSignatureStageFlag(parser, outStageFlags))
            return false;
    }
    return true;
}

// Parse unsigned integral number, e.g. "123" or "0xFF".
static int ParseUInt32(Parser& parser, std::uint32_t& outValue)
{
    const int base = parser.MatchNumeric();
    if (base == 0)
    {
        ReturnWithParseError(parser, "expected numeric value");
        return 0;
    }

    StringView tok = parser.Accept();
    outValue = 0;

    if (base == 16)
    {
        tok = tok.substr(2);
        for (char c : tok)
        {
            outValue *= 16;
            if (IsCharNumeric(c))
                outValue += (c - '0');
            else if (c >= 'a' && c <= 'f')
                outValue += (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F')
                outValue += (c - 'A' + 10);
        }
    }
    else
    {
        for (char c : tok)
        {
            outValue *= 10;
            outValue += (c - '0');
        }
    }

    return base;
}

// Parse floating-point number, e.g. "5" or "25.13".
static bool ParseFloat(Parser& parser, float& outValue)
{
    bool            hasSign     = false;
    std::uint32_t   intPart     = 0;
    std::uint32_t   fracPart    = 0;
    std::size_t     fracPartLen = 0;

    /* Parse optional sign */
    if (parser.Accept("-"))
        hasSign = true;

    /* Parse integral part */
    const int intPartBase = ParseUInt32(parser, intPart);
    if (intPartBase == 0)
        return false;
    if (intPartBase != 10)
        return ReturnWithParseError(parser, "hexadecimal number not accepted for floating-points");

    /* Parse optional fractional part */
    if (parser.Accept("."))
    {
        fracPartLen = parser.Token().size();
        const int fracPartBase = ParseUInt32(parser, fracPart);
        if (fracPartBase == 0)
            return false;
        if (fracPartBase != 10)
            return ReturnWithParseError(parser, "hexadecimal number not accepted for floating-points");
    }

    /* Convert to floating-point number */
    outValue = static_cast<float>(intPart);

    if (fracPartLen > 0)
        outValue += static_cast<float>(fracPart) / std::pow(10.0f, static_cast<float>(fracPartLen));

    if (hasSign)
        outValue = -outValue;

    return true;
}

// Parse boolean value from a set of accepted keywords.
static bool ParseBoolean(Parser& parser, bool& outValue)
{
    const StringView tok = parser.Accept();
    if (tok.empty())
        return ReturnWithParseError(parser, "expected boolean value");
    if (tok == "true" || tok == "yes" || tok == "on" || tok == "1")
        outValue = true;
    else if (tok == "false" || tok == "no" || tok == "off" || tok == "0")
        outValue = false;
    else
        return ReturnWithParseError(parser, "unknown boolean value: %s", tok);
    return true;
}

static bool ParseLayoutSignatureResourceBinding(Parser& parser, PipelineLayoutDescriptor& outDesc, bool isHeap)
{
    BindingDescriptor bindingDesc;
    bindingDesc.stageFlags = StageFlags::AllStages;

    /* Parse resource type and set stages to default */
    if (!ParseLayoutSignatureResourceType(parser, bindingDesc.type, bindingDesc.bindFlags))
        return false;

    /* Parse binding points */
    if (!parser.Accept("("))
        return ReturnWithParseError(parser, "expected open bracket '(' after resource type");

    std::vector<BindingDescriptor> intermediateBindings;

    while (parser.Feed() && !parser.Match(")"))
    {
        /* Parse optional name */
        if (parser.MatchIdent())
        {
            StringView tok = parser.Accept();
            bindingDesc.name = std::string{ tok.begin(), tok.end() };
            if (!parser.Accept("@"))
                return ReturnWithParseError(parser, "expected '@' token after resource identifier: %s", bindingDesc.name);
        }
        else
            bindingDesc.name.clear();

        /* Parse slot number */
        if (ParseUInt32(parser, bindingDesc.slot.index) == 0)
            return false;

        /* Parse optional array size */
        if (parser.Accept("["))
        {
            if (ParseUInt32(parser, bindingDesc.arraySize) == 0)
                return false;
            if (!parser.Accept("]"))
                return ReturnWithParseError(parser, "expected closing ']' after array size");
        }

        /* Add new binding point to output descriptor */
        intermediateBindings.push_back(bindingDesc);

        if (parser.Match(","))
            parser.Accept();
        else
            break;
    }

    if (!parser.Accept(")"))
        return ReturnWithParseError(parser, "expected close bracket ')' after slot indices");

    /* Parse optional static sampler */
    bool isStaticSampler = false;
    StaticSamplerDescriptor staticSamplerDesc;

    if (parser.Accept("{"))
    {
        if (bindingDesc.type == ResourceType::Sampler)
        {
            /* Parse descriptor for static sampler */
            isStaticSampler = true;

            Parser subParser;
            if (!parser.Fork("}", subParser))
                return false;

            if (!ParseSamplerDesc(subParser, staticSamplerDesc.sampler))
                return false;
        }
        else
            return ReturnWithParseError(parser, "braced initialization only supported for static samplers");
    }

    /* Parse optional shader stage flags */
    long stageFlags = StageFlags::AllStages;

    if (parser.Match(":"))
    {
        stageFlags = 0;
        if (!ParseLayoutSignatureStageFlagsAll(parser, stageFlags))
            return false;
    }

    for (BindingDescriptor& binding : intermediateBindings)
        binding.stageFlags = stageFlags;

    /* Append new binding points into output descriptor */
    if (isStaticSampler)
    {
        if (isHeap)
            return ReturnWithParseError(parser, "cannot have static sampler in heap binding layout");

        /* Append bindings to static samplers */
        outDesc.staticSamplers.reserve(outDesc.staticSamplers.size() + intermediateBindings.size());
        for (BindingDescriptor& binding : intermediateBindings)
        {
            staticSamplerDesc.name          = binding.name;
            staticSamplerDesc.stageFlags    = binding.stageFlags;
            staticSamplerDesc.slot          = binding.slot;
            outDesc.staticSamplers.push_back(staticSamplerDesc);
        }
    }
    else
    {
        /* Append bindings to heap or dynamic resource bindings */
        auto& dstBindings = (isHeap ? outDesc.heapBindings : outDesc.bindings);
        dstBindings.insert(dstBindings.end(), intermediateBindings.begin(), intermediateBindings.end());
    }

    return true;
}

static UniformType StringToUniformType(StringView s)
{
    if (s.compare(0, 5, "float") == 0)
    {
        s = s.substr(5);
        if (s == "1" || s.empty()) { return UniformType::Float1;    }
        if (s == "2")              { return UniformType::Float2;    }
        if (s == "3")              { return UniformType::Float3;    }
        if (s == "4")              { return UniformType::Float4;    }
        if (s == "2x2")            { return UniformType::Float2x2;  }
        if (s == "2x3")            { return UniformType::Float2x3;  }
        if (s == "2x4")            { return UniformType::Float2x4;  }
        if (s == "3x2")            { return UniformType::Float3x2;  }
        if (s == "3x3")            { return UniformType::Float3x3;  }
        if (s == "3x4")            { return UniformType::Float3x4;  }
        if (s == "4x2")            { return UniformType::Float4x2;  }
        if (s == "4x3")            { return UniformType::Float4x3;  }
        if (s == "4x4")            { return UniformType::Float4x4;  }
    }
    else if (s.compare(0, 6, "double") == 0)
    {
        s = s.substr(6);
        if (s == "1" || s.empty()) { return UniformType::Double1;   }
        if (s == "2")              { return UniformType::Double2;   }
        if (s == "3")              { return UniformType::Double3;   }
        if (s == "4")              { return UniformType::Double4;   }
        if (s == "2x2")            { return UniformType::Double2x2; }
        if (s == "2x3")            { return UniformType::Double2x3; }
        if (s == "2x4")            { return UniformType::Double2x4; }
        if (s == "3x2")            { return UniformType::Double3x2; }
        if (s == "3x3")            { return UniformType::Double3x3; }
        if (s == "3x4")            { return UniformType::Double3x4; }
        if (s == "4x2")            { return UniformType::Double4x2; }
        if (s == "4x3")            { return UniformType::Double4x3; }
        if (s == "4x4")            { return UniformType::Double4x4; }
    }
    else if (s.compare(0, 3, "int") == 0)
    {
        s = s.substr(3);
        if (s == "1" || s.empty()) { return UniformType::Int1;      }
        if (s == "2")              { return UniformType::Int2;      }
        if (s == "3")              { return UniformType::Int3;      }
        if (s == "4")              { return UniformType::Int4;      }
    }
    else if (s.compare(0, 4, "uint") == 0)
    {
        s = s.substr(4);
        if (s == "1" || s.empty()) { return UniformType::UInt1;     }
        if (s == "2")              { return UniformType::UInt2;     }
        if (s == "3")              { return UniformType::UInt3;     }
        if (s == "4")              { return UniformType::UInt4;     }
    }
    else if (s.compare(0, 4, "bool") == 0)
    {
        s = s.substr(4);
        if (s == "1" || s.empty()) { return UniformType::Bool1;     }
        if (s == "2")              { return UniformType::Bool2;     }
        if (s == "3")              { return UniformType::Bool3;     }
        if (s == "4")              { return UniformType::Bool4;     }
    }
    return UniformType::Undefined;
}

static bool ParseLayoutSignatureBarrierFlag(Parser& parser, PipelineLayoutDescriptor& outDesc)
{
    if (parser.Match("rw"))
    {
        parser.Accept();
        outDesc.barrierFlags |= BarrierFlags::Storage;
        return true;
    }
    else if (parser.Match("rwbuffer"))
    {
        parser.Accept();
        outDesc.barrierFlags |= BarrierFlags::StorageBuffer;
        return true;
    }
    else if (parser.Match("rwtexture"))
    {
        parser.Accept();
        outDesc.barrierFlags |= BarrierFlags::StorageTexture;
        return true;
    }
    return ReturnWithParseError(parser, "unknown barrier flag: %s", parser.Token());
}

static bool ParseLayoutSignatureBinding(Parser& parser, PipelineLayoutDescriptor& outDesc, bool isHeap)
{
    /* Check if resource type denotes a uniform binding */
    const UniformType uniformType = StringToUniformType(parser.Token());
    if (uniformType != UniformType::Undefined)
    {
        if (isHeap)
            return ReturnWithParseError(parser, "uniform bindings must not be declared inside a heap");

        parser.Accept();

        if (!parser.Accept("("))
            return ReturnWithParseError(parser, "expected open bracket '(' after resource type");

        while (parser.Feed() && !parser.Match(")"))
        {
            UniformDescriptor uniformDesc;
            uniformDesc.type = uniformType;

            /* Parse uniform name */
            if (!parser.MatchIdent())
                return ReturnWithParseError(parser, "expected uniform name");

            const StringView uniformName = parser.Accept();
            uniformDesc.name = std::string{ uniformName.begin(), uniformName.end() };

            /* Parse optional array size */
            if (parser.Accept("["))
            {
                if (ParseUInt32(parser, uniformDesc.arraySize) == 0)
                    return false;
                if (!parser.Accept("]"))
                    return ReturnWithParseError(parser, "expected close squared bracket ']' after array size");
            }

            /* Append current uniform descriptor to output layout */
            outDesc.uniforms.push_back(uniformDesc);

            if (parser.Match(","))
                parser.Accept();
            else
                break;
        }

        if (!parser.Accept(")"))
            return ReturnWithParseError(parser, "expected close bracket ')' after uniform descriptor");

        return true;
    }

    /* Check if resource type denotes a barrier bitmask */
    if (parser.Match("barriers"))
    {
        if (isHeap)
            return ReturnWithParseError(parser, "barrier flags must not be declared inside a heap");

        parser.Accept();

        if (!parser.Accept("{"))
            return ReturnWithParseError(parser, "expected open curly bracket '{' after barrier flags");

        while (parser.Feed() && !parser.Match("}"))
        {
            /* Parse next barrier flag */
            if (!ParseLayoutSignatureBarrierFlag(parser, outDesc))
                return false;

            /* If there's no comma, the flags block must end */
            if (parser.Match(","))
                parser.Accept();
            else
                break;
        }

        if (!parser.Accept("}"))
            return ReturnWithParseError(parser, "expected closing curly bracket '}' after end of barrier flags");

        return true;
    }

    /* Otherwise, parse resource binding */
    return ParseLayoutSignatureResourceBinding(parser, outDesc, isHeap);
}

static bool ParseLayoutSignatureForHeap(Parser& parser, PipelineLayoutDescriptor& outDesc)
{
    if (!parser.Accept("{"))
        return ReturnWithParseError(parser, "expected open curly bracket '{' after heap declaration");

    while (parser.Feed() && !parser.Match("}"))
    {
        /* Parse next binding point */
        if (!ParseLayoutSignatureBinding(parser, outDesc, /*isHeap:*/ true))
            return false;

        /* If there's no comma, the layout must end */
        if (parser.Match(","))
            parser.Accept();
        else
            break;
    }

    if (!parser.Accept("}"))
        return ReturnWithParseError(parser, "expected closing curly bracket '}' after end of heap declaration");

    return true;
}

static bool ParsePipelineLayoutDesc(Parser& parser, PipelineLayoutDescriptor& outDesc)
{
    while (parser.Feed())
    {
        if (parser.Match("heap"))
        {
            /* Parse heap layout bindings */
            parser.Accept();
            if (!ParseLayoutSignatureForHeap(parser, outDesc))
                return false;
        }
        else
        {
            /* Parse next binding point */
            if (!ParseLayoutSignatureBinding(parser, outDesc, /*isHeap:*/ false))
                return false;
        }

        /* If there's no comma, the descriptor must end */
        if (!parser.Accept(",") && parser.Feed())
            return ReturnWithParseError(parser, "expected comma separator ',' after binding point");
    }
    return true;
}

[[noreturn]]
static void RaiseParsingError(const Parser& parser, const char* descName)
{
    if (parser.report.HasErrors())
    {
        /* Raise parser report */
        LLGL_TRAP("parsing %s failed: %s", descName, parser.report.GetText());
    }
    else
    {
        /* Raise token error */
        StringView errorToken = parser.Token();
        const std::string errorTokenStr{ errorToken.begin(), errorToken.end() };
        LLGL_TRAP("parsing %s failed at token '%s'", descName, errorTokenStr.c_str());
    }
}

PipelineLayoutDescriptor ParseContext::AsPipelineLayoutDesc() const
{
    PipelineLayoutDescriptor desc;
    Parser parser{ tokens_ };
    if (!ParsePipelineLayoutDesc(parser, desc))
        RaiseParsingError(parser, "PipelineLayoutDescriptor");
    return desc;
}

static bool ParseSamplerDescAddress(Parser& parser, SamplerDescriptor& outDesc)
{
    /* Parse optional address mode axes (by default UVW) */
    int axes = 0x1 | 0x2 | 0x4;

    if (parser.Accept("."))
    {
        auto EnableAxis = [&axes, &parser](int axis, const char* axisName, const StringView& tok) -> bool
        {
            if ((axes & axis) != 0)
            {
                std::string tokStr{ tok.begin(), tok.end() };
                parser.report.Errorf("duplicate sampler address mode %s axis: %s", axisName, tokStr.c_str());
                return false;
            }
            axes |= axis;
            return true;
        };

        axes = 0;

        const StringView tok = parser.Accept();
        if (tok.empty())
            return ReturnWithParseError(parser, "expected sampler address mode axes after '.' punctuation");

        for (char c : tok)
        {
            switch (c)
            {
                case 'u': case 'x':
                    if (!EnableAxis(0x1, "X", tok))
                        return false;
                    break;

                case 'v': case 'y':
                    if (!EnableAxis(0x2, "Y", tok))
                        return false;
                    break;

                case 'w': case 'z':
                    if (!EnableAxis(0x4, "Z", tok))
                        return false;
                    break;

                default:
                    return ReturnWithParseError(parser, "unknown sampler address mode axis: %s", tok);
            }
        }
    }

    /* Parse attribute initializer value */
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected initializer after sampler attribute");

    struct ModeIdentValuePair
    {
        const char*         ident;
        SamplerAddressMode  address;
    };

    constexpr ModeIdentValuePair acceptedModes[] =
    {
        { "repeat",     SamplerAddressMode::Repeat     },
        { "mirror",     SamplerAddressMode::Mirror     },
        { "clamp",      SamplerAddressMode::Clamp      },
        { "border",     SamplerAddressMode::Border     },
        { "mirrorOnce", SamplerAddressMode::MirrorOnce },
    };

    StringView tok = parser.Token();
    if (tok.empty())
        return ReturnWithParseError(parser, "expected sampler address mode");

    /* Set output attribute */
    for (const ModeIdentValuePair& mode : acceptedModes)
    {
        if (tok == mode.ident)
        {
            parser.Accept();
            if ((axes & 0x1) != 0)
                outDesc.addressModeU = mode.address;
            if ((axes & 0x2) != 0)
                outDesc.addressModeV = mode.address;
            if ((axes & 0x4) != 0)
                outDesc.addressModeW = mode.address;
            return true;
        }
    }

    /* Unknown sampler address mode */
    return ReturnWithParseError(parser, "unknown sampler address mode: %s", tok);
}

static bool ParseSamplerDescFilter(Parser& parser, SamplerDescriptor& outDesc)
{
    /* Parse optional filter type */
    int filters = 0x1 | 0x2 | 0x4;

    if (parser.Accept("."))
    {
        filters = 0;

        const StringView tok = parser.Accept();
        if (tok.empty())
            return ReturnWithParseError(parser, "expected sampler filter type after '.' punctuation");

        if (tok == "min")
            filters |= 0x1;
        else if (tok == "mag")
            filters |= 0x2;
        else if (tok == "mip")
            filters |= 0x4;
        else
            return ReturnWithParseError(parser, "unknown sampler filter: %s", tok);
    }

    /* Parse attribute initializer value */
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected initializer after sampler attribute");

    struct FilterIdentValuePair
    {
        const char*     ident;
        SamplerFilter   value;
    };

    constexpr FilterIdentValuePair acceptedFilters[] =
    {
        { "nearest", SamplerFilter::Nearest },
        { "linear",  SamplerFilter::Linear  },
    };

    StringView tok = parser.Accept();
    if (tok.empty())
        return ReturnWithParseError(parser, "expected sampler filter");

    /* Set output attribute */
    for (const FilterIdentValuePair& filter : acceptedFilters)
    {
        if (tok == filter.ident)
        {
            if ((filters & 0x1) != 0)
                outDesc.minFilter = filter.value;
            if ((filters & 0x2) != 0)
                outDesc.magFilter = filter.value;
            if ((filters & 0x4) != 0)
            {
                outDesc.mipMapFilter = filter.value;
                outDesc.mipMapEnabled = true;
            }
            return true;
        }
    }

    /* Check for special case to disable MIP-mapping */
    if (tok == "none")
    {
        if (filters == 0x4)
        {
            outDesc.mipMapEnabled = false;
            return true;
        }
        else
            return ReturnWithParseError(parser, "sampler filter 'none' can only be used for MIP-map filter");
    }

    /* Unknown sampler address mode */
    return ReturnWithParseError(parser, "unknown sampler address mode: %s", tok);
}

static bool ParseSamplerDescLod(Parser& parser, SamplerDescriptor& outDesc)
{
    if (!parser.Accept("."))
        return ReturnWithParseError(parser, "expected '.' punctuation after sampler 'lod' attribute");

    const StringView tok = parser.Accept();
    if (tok.empty())
        return ReturnWithParseError(parser, "expected lod attribute after '.' punctuation");

    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after lod attribute");

    if (tok == "bias")
    {
        if (!ParseFloat(parser, outDesc.mipMapLODBias))
            return false;
    }
    else if (tok == "min")
    {
        if (!ParseFloat(parser, outDesc.minLOD))
            return false;
    }
    else if (tok == "max")
    {
        if (!ParseFloat(parser, outDesc.maxLOD))
            return false;
    }
    else
        return ReturnWithParseError(parser, "unknown sampler lod attribute: %s", tok);

    return true;
}

static bool ParseSamplerDescAnisotropy(Parser& parser, SamplerDescriptor& outDesc)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after anisotropy attribute");
    return (ParseUInt32(parser, outDesc.maxAnisotropy) != 0);
}

static bool ParseCompareOp(Parser& parser, CompareOp& outValue)
{
    return ParseValueFromDictionary<CompareOp>(
        parser,
        {
            { "never",  CompareOp::NeverPass    },
            { "ls",     CompareOp::Less         },
            { "eq",     CompareOp::Equal        },
            { "le",     CompareOp::LessEqual    },
            { "gr",     CompareOp::Greater      },
            { "ne",     CompareOp::NotEqual     },
            { "ge",     CompareOp::GreaterEqual },
            { "always", CompareOp::AlwaysPass   },
        },
        outValue,
        "compare operator"
    );
}

static bool ParseStencilOp(Parser& parser, StencilOp& outValue)
{
    return ParseValueFromDictionary<StencilOp>(
        parser,
        {
            { "keep", StencilOp::Keep     },
            { "zero", StencilOp::Zero     },
            { "set",  StencilOp::Replace  },
            { "inc",  StencilOp::IncClamp },
            { "dec",  StencilOp::DecClamp },
            { "inv",  StencilOp::Invert   },
            { "incw", StencilOp::IncWrap  },
            { "decw", StencilOp::DecWrap  },
        },
        outValue,
        "stencil operator"
    );
}

static bool ParseSamplerDescCompare(Parser& parser, SamplerDescriptor& outDesc)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after compare attribute");

    outDesc.compareEnabled = true;

    return ParseCompareOp(parser, outDesc.compareOp);
}

static bool ParseSamplerDescBorder(Parser& parser, SamplerDescriptor& outDesc)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after border attribute");

    const StringView tok = parser.Accept();
    if (tok.empty())
        return ReturnWithParseError(parser, "expected border color");

    if (tok == "transparent")
    {
        outDesc.borderColor[0] = 0.0f;
        outDesc.borderColor[1] = 0.0f;
        outDesc.borderColor[2] = 0.0f;
        outDesc.borderColor[3] = 0.0f;
    }
    else if (tok == "black")
    {
        outDesc.borderColor[0] = 0.0f;
        outDesc.borderColor[1] = 0.0f;
        outDesc.borderColor[2] = 0.0f;
        outDesc.borderColor[3] = 1.0f;
    }
    else if (tok == "white")
    {
        outDesc.borderColor[0] = 1.0f;
        outDesc.borderColor[1] = 1.0f;
        outDesc.borderColor[2] = 1.0f;
        outDesc.borderColor[3] = 1.0f;
    }
    else
        return ReturnWithParseError(parser, "unknown border color: %s", tok);

    return true;
}

static bool ParseSamplerDesc(Parser& parser, SamplerDescriptor& outDesc)
{
    while (parser.Feed())
    {
        /* Parse sampler attribute identifier */
        if (!parser.MatchIdent())
            return ReturnWithParseError(parser, "expected identifier for sampler attribute");
        const StringView tok = parser.Accept();

        if (tok == "address")
        {
            if (!ParseSamplerDescAddress(parser, outDesc))
                return false;
        }
        else if (tok == "filter")
        {
            if (!ParseSamplerDescFilter(parser, outDesc))
                return false;
        }
        else if (tok == "lod")
        {
            if (!ParseSamplerDescLod(parser, outDesc))
                return false;
        }
        else if (tok == "anisotropy")
        {
            if (!ParseSamplerDescAnisotropy(parser, outDesc))
                return false;
        }
        else if (tok == "compare")
        {
            if (!ParseSamplerDescCompare(parser, outDesc))
                return false;
        }
        else if (tok == "border")
        {
            if (!ParseSamplerDescBorder(parser, outDesc))
                return false;
        }
        else
            return ReturnWithParseError(parser, "unknown sampler attribute: %s", tok);

        /* If there's no comma, the descriptor must end */
        if (!parser.Accept(",") && parser.Feed())
            return ReturnWithParseError(parser, "expected comma separator ',' after sampler attribute");
    }
    return true;
}

SamplerDescriptor ParseContext::AsSamplerDesc() const
{
    SamplerDescriptor desc;
    Parser parser{ tokens_ };
    if (!ParseSamplerDesc(parser, desc))
        RaiseParsingError(parser, "SamplerDescriptor");
    return desc;
}

static bool ParseDepthDescCompare(Parser& parser, DepthDescriptor& outDesc)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after depth compare attribute");
    return ParseCompareOp(parser, outDesc.compareOp);
}

static bool ParseDepthDescBoolean(Parser& parser, bool& outValue)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after depth attribute");
    return ParseBoolean(parser, outValue);
}

static bool ParseDepthDesc(Parser& parser, DepthDescriptor& outDesc)
{
    while (parser.Feed())
    {
        /* Parse sampler attribute identifier */
        if (!parser.MatchIdent())
            return ReturnWithParseError(parser, "expected identifier for depth attribute");
        const StringView tok = parser.Accept();

        if (tok == "compare")
        {
            if (!ParseDepthDescCompare(parser, outDesc))
                return false;
        }
        else if (tok == "test")
        {
            if (!ParseDepthDescBoolean(parser, outDesc.testEnabled))
                return false;
        }
        else if (tok == "write")
        {
            if (!ParseDepthDescBoolean(parser, outDesc.writeEnabled))
                return false;
        }

        /* If there's no comma, the descriptor must end */
        if (!parser.Accept(",") && parser.Feed())
            return ReturnWithParseError(parser, "expected comma separator ',' after depth attribute");
    }
    return true;
}

DepthDescriptor ParseContext::AsDepthDesc() const
{
    DepthDescriptor desc;
    Parser parser{ tokens_ };
    if (!ParseDepthDesc(parser, desc))
        RaiseParsingError(parser, "DepthDescriptor");
    return desc;
}

static bool ParseStencilFaceDescStencilOp(Parser& parser, StencilOp& outValue)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after stencil-face operator attribute");
    return ParseStencilOp(parser, outValue);
}

static bool ParseStencilFaceDescCompare(Parser& parser, StencilFaceDescriptor& outDesc)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after stencil-face compare attribute");
    return ParseCompareOp(parser, outDesc.compareOp);
}

static bool ParseStencilFaceDescUInt32(Parser& parser, std::uint32_t& outValue)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after stencil-face attribute");
    return (ParseUInt32(parser, outValue) != 0);
}

static bool ParseStencilFaceDescRef(Parser& parser, std::uint32_t& outReference, bool& outDynamicReference)
{
    if (!parser.Accept("="))
        return ReturnWithParseError(parser, "expected assignment '=' after stencil-face reference attribute");
    if (parser.Accept("dynamic"))
    {
        outDynamicReference = true;
        return true;
    }
    return (ParseUInt32(parser, outReference) != 0);
}

static bool ParseStencilFaceDescAttribute(Parser& parser, const StringView& tok, StencilFaceDescriptor& outDesc, StencilDescriptor* outStencilDesc = nullptr)
{
    if (tok == "sfail")
    {
        if (!ParseStencilFaceDescStencilOp(parser, outDesc.stencilFailOp))
            return false;
    }
    else if (tok == "dfail")
    {
        if (!ParseStencilFaceDescStencilOp(parser, outDesc.depthFailOp))
            return false;
    }
    else if (tok == "dpass")
    {
        if (!ParseStencilFaceDescStencilOp(parser, outDesc.depthPassOp))
            return false;
    }
    else if (tok == "compare")
    {
        if (!ParseStencilFaceDescCompare(parser, outDesc))
            return false;
    }
    else if (tok == "read")
    {
        if (!ParseStencilFaceDescUInt32(parser, outDesc.readMask))
            return false;
    }
    else if (tok == "write")
    {
        if (!ParseStencilFaceDescUInt32(parser, outDesc.writeMask))
            return false;
    }
    else if (tok == "ref")
    {
        if (outStencilDesc)
        {
            if (!ParseStencilFaceDescRef(parser, outDesc.reference, outStencilDesc->referenceDynamic))
                return false;
        }
        else
        {
            if (!ParseStencilFaceDescUInt32(parser, outDesc.reference))
                return false;
        }
    }
    else
    {
        if (outStencilDesc != nullptr)
            return ReturnWithParseError(parser, "unknown stencil attribute: %s", tok);
        else
            return ReturnWithParseError(parser, "unknown stencil-face attribute: %s", tok);
    }

    return true;
}

static bool ParseStencilFaceDesc(Parser& parser, StencilFaceDescriptor& outDesc)
{
    while (parser.Feed())
    {
        /* Parse stencil-face attribute identifier */
        if (!parser.MatchIdent())
            return ReturnWithParseError(parser, "expected identifier for stencil-face attribute");
        const StringView tok = parser.Accept();

        if (!ParseStencilFaceDescAttribute(parser, tok, outDesc))
            return false;

        /* If there's no comma, the descriptor must end */
        if (!parser.Accept(",") && parser.Feed())
            return ReturnWithParseError(parser, "expected comma separator ',' after stencil-face attribute");
    }
    return true;
}

StencilFaceDescriptor ParseContext::AsStencilFaceDesc() const
{
    StencilFaceDescriptor desc;
    Parser parser{ tokens_ };
    if (!ParseStencilFaceDesc(parser, desc))
        RaiseParsingError(parser, "StencilFaceDescriptor");
    return desc;
}

static bool ParseStencilDesc(Parser& parser, StencilDescriptor& outDesc)
{
    bool hasIndependentFaces = false;
    bool hasUniformFaces = false;

    while (parser.Feed())
    {
        /* Parse sampler attribute identifier */
        if (!parser.MatchIdent())
            return ReturnWithParseError(parser, "expected identifier for stencil attribute");
        const StringView tok = parser.Accept();

        if (tok == "test")
        {
            if (!ParseDepthDescBoolean(parser, outDesc.testEnabled))
                return false;
        }
        else if (tok == "front" || tok == "back")
        {
            /* Only use either independent or uniform stencil faces */
            if (hasUniformFaces)
                return ReturnWithParseError(parser, "cannot continue with independent stencil faces after uniform stencil faces");
            hasIndependentFaces = true;

            const bool isBackFace = (tok == "back");

            /* Parse sub section for explicit front face */
            if (!parser.Accept("{"))
                return ReturnWithParseError(parser, "expected open curly bracket '{' after stencil face declaration");

            Parser subParser;
            if (!parser.Fork("}", subParser))
                return false;

            if (!ParseStencilFaceDesc(subParser, (isBackFace ? outDesc.back : outDesc.front)))
                return false;
        }
        else
        {
            /* Only use either independent or uniform stencil faces */
            if (hasIndependentFaces)
                return ReturnWithParseError(parser, "cannot continue with uniform stencil faces after independent stencil faces");
            hasUniformFaces = true;

            /* Parse attributes for implicit front face */
            if (!ParseStencilFaceDescAttribute(parser, tok, outDesc.front, &outDesc))
                return false;
        }

        /* If there's no comma, the descriptor must end */
        if (!parser.Accept(",") && parser.Feed())
            return ReturnWithParseError(parser, "expected comma separator ',' after stencil attribute");
    }

    if (hasUniformFaces)
        outDesc.back = outDesc.front;

    return true;
}

StencilDescriptor ParseContext::AsStencilDesc() const
{
    StencilDescriptor desc;
    Parser parser{ tokens_ };
    if (!ParseStencilDesc(parser, desc))
        RaiseParsingError(parser, "StencilDescriptor");
    return desc;
}

static TextureSwizzle ParseTextureSwizzle(const char* s, char c)
{
    switch (c)
    {
        case '1':           return TextureSwizzle::One;
        case '0':           return TextureSwizzle::Zero;
        case 'r': case 'R': return TextureSwizzle::Red;
        case 'g': case 'G': return TextureSwizzle::Green;
        case 'b': case 'B': return TextureSwizzle::Blue;
        case 'a': case 'A': return TextureSwizzle::Alpha;
        default:
            LLGL_TRAP("parsing %s failed: invalid character", s);
    }
}

TextureSwizzleRGBA ParseContext::AsTextureSwizzleRGBA() const
{
    TextureSwizzleRGBA swizzle;
    if (!(tokens_.size() == 1 && tokens_.front().size() == 4))
        LLGL_TRAP("parsing %s failed: texture swizzle must consist of four characters in {1,2,R,G,B,A}", source_.data());
    const char* tok = tokens_.front().data();
    swizzle.r = ParseTextureSwizzle(tok, tok[0]);
    swizzle.g = ParseTextureSwizzle(tok, tok[1]);
    swizzle.b = ParseTextureSwizzle(tok, tok[2]);
    swizzle.a = ParseTextureSwizzle(tok, tok[3]);
    return swizzle;
}


/*
 * Global functions
 */

LLGL_EXPORT ParseContext Parse(const char* format, ...)
{
    if (std::strchr(format, '%') != nullptr)
    {
        std::string s;
        LLGL_STRING_PRINTF(s, format);
        return ParseContext{ UTF8String{ s.c_str() } };
    }
    else
    {
        /* Forward string to ParseContext unmodified */
        return ParseContext{ StringView{ format } };
    }
}


} // /namespace LLGL



// ================================================================================
