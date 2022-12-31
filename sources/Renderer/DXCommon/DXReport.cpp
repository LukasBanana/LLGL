/*
 * DXReport.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DXReport.h"
#include "DXCore.h"


namespace LLGL
{


void DXReport::Reset(const StringView& text, bool hasErrors)
{
    text_       = std::string(text.begin(), text.end());
    hasErrors_  = hasErrors;
}

void DXReport::Reset(ID3DBlob* blob, bool hasErrors)
{
    if (blob != nullptr)
        text_ = DXGetBlobString(blob);
    hasErrors_ = hasErrors;
}

const char* DXReport::GetText() const
{
    return text_.c_str();
}

bool DXReport::HasErrors() const
{
    return hasErrors_;
}


} // /namespace LLGL



// ================================================================================
