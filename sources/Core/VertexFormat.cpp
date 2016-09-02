/*
 * VertexFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/VertexFormat.h>
#include "TypeConversion.h"
#include <stdexcept>
#include <sstream>


namespace LLGL
{


void VertexFormat::AddAttribute(
    const std::string& name, const DataType dataType, unsigned int components, bool conversion)
{
    AddAttribute(name, 0, dataType, components, conversion);
}

void VertexFormat::AddAttribute(
    const std::string& semanticName, unsigned int semanticIndex, const DataType dataType, unsigned int components, bool conversion)
{
    if (components < 1 || components > 4)
    {
        std::stringstream s;
        s << __FUNCTION__ << ": 'components' argument must be 1, 2, 3, or 4 but ( " << components << " ) is specified";
        throw std::invalid_argument(s.str());
    }

    /* Setup new vertex attribute */
    VertexAttribute attrib;
    {
        attrib.dataType         = dataType;
        attrib.conversion       = conversion;
        attrib.components       = components;
        attrib.offset           = formatSize_;
        attrib.name             = semanticName;
        attrib.semanticIndex    = semanticIndex;
    }
    attributes_.push_back(attrib);

    /* Increase format size */
    formatSize_ += (GetDataTypeSize(dataType) * components);
}


} // /namespace LLGL



// ================================================================================
