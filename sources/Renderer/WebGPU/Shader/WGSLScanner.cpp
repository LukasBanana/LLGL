/*
 * WGSLScanner.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGSLScanner.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Ascii.h"
#include "../../../Core/Exception.h"
#include "../../../Core/StringUtils.h"
#include <LLGL/Container/Strings.h>
#include <regex>
#include <string>


namespace LLGL
{


using namespace Ascii;

static void ScanWGSLTokensInternal(const char* start, const char* end, DynamicVector<WGSLToken>& outTokens)
{
    /* Stores the current token */
    auto NextTokenSpell = [&start](const char* s) -> StringView
    {
        return StringView{ start, static_cast<std::size_t>(s - start) };
    };

    auto NextToken = [NextTokenSpell, &outTokens](WGSLTokenType type, const char* s)
    {
        outTokens.push_back(WGSLToken{ type, NextTokenSpell(s) });
    };

    /*
    WGSL grammar: https://www.w3.org/TR/WGSL/#numeric-literals
    */

    /*
    int_literal
      : decimal_int_literal
      | hex_int_literal
    decimal_int_literal
      : /0[iu]?/
      | /[1-9][0-9]*[iu]?/
    hex_int_literal
      : /0[xX][0-9a-fA-F]+[iu]?/
    */
    static const std::regex kIntLiteralRegex
    {
        R"(^(?:0[xX][0-9a-fA-F]+|0|[1-9][0-9]*)[iu]?)"
    };

    /*
    float_literal
      : decimal_float_literal
      | hex_float_literal
    decimal_float_literal
      : /0[fh]/
      | /[1-9][0-9]*[fh]/
      | /[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?[fh]?/
      | /[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?[fh]?/
      | /[0-9]+[eE][+-]?[0-9]+[fh]?/
    hex_float_literal
      : /0[xX][0-9a-fA-F]*\.[0-9a-fA-F]+([pP][+-]?[0-9]+[fh]?)?/
      | /0[xX][0-9a-fA-F]+\.[0-9a-fA-F]*([pP][+-]?[0-9]+[fh]?)?/
      | /0[xX][0-9a-fA-F]+[pP][+-]?[0-9]+[fh]?/
    */
    const std::string kHexFloat         = R"(0[xX](?:[0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.[0-9a-fA-F]*|[0-9a-fA-F]+)[pP][+-]?[0-9]+[fh]?)";
    const std::string kHexFloatNoExp    = R"(0[xX](?:[0-9a-fA-F]*\.[0-9a-fA-F]+|[0-9a-fA-F]+\.[0-9a-fA-F]*)[fh]?)";
    const std::string kDecFloat         = R"((?:[0-9]*\.[0-9]+|[0-9]+\.[0-9]*)(?:[eE][+-]?[0-9]+)?[fh]?)";
    const std::string kSciFloat         = R"([0-9]+[eE][+-]?[0-9]+[fh]?)";
    const std::string kSuffFloat        = R"([0-9]+[fh])";

    // Combine them into one anchored, non-capturing group
    static const std::regex kFloatLiteralRegex{ 
        "^(" + kHexFloat + "|" + kHexFloatNoExp + "|" + kDecFloat + "|" + kSciFloat + "|" + kSuffFloat + ")" 
    };

    for (const char* s = start; s != end; start = s)
    {
        /* Check for commentaries */
        if (IsCharWhitespace(*s))
        {
            /* Ignore whitespaces */
            while (s != end && IsCharWhitespace(*s))
                ++s;
        }
        else if ((s + 1) != end && s[0] == '/' && s[1] == '/')
        {
            /* Skip initial and ignore remainder of current line */
            s += 2;
            while (s != end && s[0] != '\n')
                ++s;
        }
        else if ((s + 1) != end && s[0] == '/' && s[1] == '*')
        {
            /* Skip initial and find end of multi-line comment and account for nested multi-line comments */
            s += 2;
            unsigned multiLineCommentLevel = 1;
            while (s + 1 < end && multiLineCommentLevel > 0)
            {
                if (s[0] == '/' && s[1] == '*')
                {
                    ++multiLineCommentLevel;
                    s += 2;
                }
                else if (s[0] == '*' && s[1] == '/')
                {
                    --multiLineCommentLevel;
                    s += 2;
                }
                else
                    ++s;
            }
        }
        else if (IsCharNumeric(*s) || *s == '.')
        {
            /* Use regex to find the longest valid integer literal starting at 's' */
            std::cmatch match;
            if (std::regex_search(s, end, match, kFloatLiteralRegex))
            {
                /* Advance 's' by the length of the match */
                s += match.length();
                NextToken(WGSLTokenType::FloatLiteral, s);
            }
            else if (std::regex_search(s, end, match, kIntLiteralRegex))
            {
                /* Advance 's' by the length of the match */
                s += match.length();
                NextToken(WGSLTokenType::IntLiteral, s);
            }
            else
            {
                /* Fallback for safety: if regex fails but IsCharNumeric was true */
                ++s;
                NextToken(WGSLTokenType::Punctuation, s);
            }
        }
        else if (IsCharIdentifier(*s))
        {
            /* Accept alpha-numeric token */
            while (s != end && IsCharIdentifier(*s))
                ++s;

            /* Add boolean literal for 'true' and 'false' strings */
            const StringView spell = NextTokenSpell(s);
            if (spell == "true" || spell == "false")
                outTokens.push_back(WGSLToken{ WGSLTokenType::BoolLiteral, spell });
            else
                outTokens.push_back(WGSLToken{ WGSLTokenType::Identifier, spell });
        }
        else
        {
            /* Accept as single character token such as punctuation (e.g. '{' or '}') */
            ++s;
            NextToken(WGSLTokenType::Punctuation, s);
        }
    }
}

void ScanWGSLTokens(StringView source, DynamicVector<WGSLToken>& outTokens)
{
    /* Reserve token array with average token length */
    constexpr std::size_t averageTokenLength = 8;
    outTokens.reserve(source.size() / averageTokenLength);
    ScanWGSLTokensInternal(source.begin(), source.end(), outTokens);
}

bool ScanWGSLIntLiteral(StringView spell, std::uint64_t& outValue, bool* outIsUnsigned, Report* outReport)
{
    #if LLGL_EXCEPTIONS_SUPPORTED
    try
    #endif
    {
        std::string s{ spell.data(), spell.size() };

        /* Scan integer literal with STL function std::stoull() */
        std::size_t endPos = 0;
        outValue = std::stoull(s, &endPos, 0);

        /* Check for optional suffixes to determine signedness */
        if (outIsUnsigned != nullptr)
            *outIsUnsigned = (endPos < s.size() && s[endPos] == 'u');
    }
    #if LLGL_EXCEPTIONS_SUPPORTED
    catch (const std::invalid_argument& e)
    {
        if (outReport != nullptr)
            outReport->Errorf("std::invalid_argument: %s\n", e.what());
        return false;
    }
    catch (const std::out_of_range& e)
    {
        if (outReport != nullptr)
            outReport->Errorf("std::out_of_range: %s\n", e.what());
        return false;
    }
    #endif
    return true;
}



} // /namespace LLGL



// ================================================================================
