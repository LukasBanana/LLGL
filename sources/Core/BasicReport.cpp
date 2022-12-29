/*
 * BasicReport.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "BasicReport.h"


namespace LLGL
{


BasicReport::BasicReport(const StringView& text, bool hasErrors) :
    text_      { text.begin(), text.end() },
    hasErrors_ { hasErrors                }
{
}

void BasicReport::Reset(const StringView& text, bool hasErrors)
{
    text_       = std::string(text.begin(), text.end());
    hasErrors_  = hasErrors;
}

void BasicReport::Reset(std::string&& text, bool hasErrors)
{
    text_       = std::move(text);
    hasErrors_  = hasErrors;
}

const char* BasicReport::GetText() const
{
    return text_.c_str();
}

bool BasicReport::HasErrors() const
{
    return hasErrors_;
}


} // /namespace LLGL



// ================================================================================
