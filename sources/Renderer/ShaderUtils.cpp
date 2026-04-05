/*
 * ShaderUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ShaderUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


ShaderSourceLine ShaderSourceContext::GetSourceLine(unsigned lineNum, unsigned markerOffset, unsigned markerLength) const
{
    ShaderSourceLine outLine;

    for (std::size_t lineStart = 0, lineEnd = 0; lineEnd < sourceText.size(); lineStart = lineEnd + 1)
    {
        lineEnd = sourceText.find('\n', lineStart);
        if (lineNum-- == 0)
        {
            outLine.lineText = sourceText.substr(lineStart, lineEnd - lineStart);
            outLine.lineMarker = (std::string(markerOffset, ' ') + '^' + (markerLength > 1 ? std::string(markerLength - 1, '~') : std::string()));
            for_range(i, std::min(markerOffset, static_cast<unsigned>(outLine.lineText.size())))
            {
                if (outLine.lineText[i] == '\t')
                    outLine.lineMarker[i] = '\t';
            }
            break;
        }
    }

    return outLine;
}


} // /namespace LLGL



// ================================================================================
