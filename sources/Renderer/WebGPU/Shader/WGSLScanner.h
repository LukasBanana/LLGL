/*
 * WGSLScanner.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SL_SCANNER_H
#define LLGL_WG_SL_SCANNER_H


#include <LLGL/Container/DynamicVector.h>
#include <LLGL/Container/StringView.h>
#include <LLGL/Report.h>
#include <cstdint>


namespace LLGL
{


enum class WGSLTokenType
{
    Identifier,
    IntLiteral,
    FloatLiteral,
    BoolLiteral,    // true | false
    Punctuation,    // . , : ; ( ) [ ] { } < > + - / *
};

struct WGSLToken
{
    WGSLTokenType   type;
    StringView      spell;
};

void ScanWGSLTokens(StringView source, DynamicVector<WGSLToken>& outTokens);

bool ScanWGSLIntLiteral(StringView spell, std::uint64_t& outValue, bool* outIsUnsigned = nullptr, Report* outReport = nullptr);


} // /namespace LLGL


#endif



// ================================================================================
