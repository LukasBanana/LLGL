/*
 * ShaderUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SHADER_UTILS_H
#define LLGL_SHADER_UTILS_H


#include <LLGL/Export.h>
#include <LLGL/Container/StringView.h>
#include <LLGL/RenderSystemFlags.h>
#include <string>


namespace LLGL
{


struct ShaderSourceLine
{
    StringView  lineText;
    std::string lineMarker;
};

struct ShaderSourceContext
{
    ShadingLanguage inputLanguage;
    StringView      sourceName;
    StringView      sourceText;

    LLGL_EXPORT ShaderSourceLine GetSourceLine(unsigned lineNum, unsigned markerOffset, unsigned markerLength) const;
};



} // /namespace LLGL


#endif



// ================================================================================
