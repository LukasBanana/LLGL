/*
 * StreamOutputFormat.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/StreamOutputFormat.h>


namespace LLGL
{


void StreamOutputFormat::AppendAttribute(const StreamOutputAttribute& attrib)
{
    /* Append attribute to the list */
    attributes.push_back(attrib);

    //TODO: overwrite attrib.outputSlot
}

void StreamOutputFormat::AppendAttributes(const StreamOutputFormat& format)
{
    for (const auto& attr : format.attributes)
        AppendAttribute(attr);
}


} // /namespace LLGL



// ================================================================================
