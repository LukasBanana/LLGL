/*
 * Report.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Report.h>
#include <string>
#include <stdarg.h>
#include "StringUtils.h"


namespace LLGL
{


struct Report::Pimpl
{
    std::string text;
    bool        hasErrors;
};

Report::Report() :
    pimpl_ { nullptr }
{
}

Report::Report(const char* text, bool hasErrors) :
    pimpl_ { new Report::Pimpl{ text, hasErrors } }
{
}

Report::Report(const StringView& text, bool hasErrors) :
    pimpl_ { new Report::Pimpl{ std::string(text.begin(), text.end()), hasErrors } }
{
}

Report::Report(std::string&& text, bool hasErrors) :
    pimpl_ { new Report::Pimpl{ std::move(text), hasErrors } }
{
}

Report::Report(const Report& rhs) :
    pimpl_ { rhs.pimpl_ != nullptr ? new Report::Pimpl{ *rhs.pimpl_ } : nullptr }
{
}

Report::Report(Report&& rhs) noexcept :
    pimpl_ { rhs.pimpl_ }
{
    rhs.pimpl_ = nullptr;
}

Report::~Report()
{
    delete pimpl_;
}

const char* Report::GetText() const
{
    return (pimpl_ != nullptr ? pimpl_->text.c_str() : "");
}

bool Report::HasErrors() const
{
    return (pimpl_ != nullptr && pimpl_->hasErrors);
}

void Report::Reset(const char* text, bool hasErrors)
{
    if (pimpl_ != nullptr)
    {
        pimpl_->text        = text;
        pimpl_->hasErrors   = hasErrors;
    }
    else
        pimpl_ = new Report::Pimpl{ text, hasErrors };
}

void Report::Reset(const StringView& text, bool hasErrors)
{
    if (pimpl_ != nullptr)
    {
        pimpl_->text        = std::string(text.begin(), text.end());
        pimpl_->hasErrors   = hasErrors;
    }
    else
        pimpl_ = new Report::Pimpl{ std::string(text.begin(), text.end()), hasErrors };
}

void Report::Reset(std::string&& text, bool hasErrors)
{
    if (pimpl_ != nullptr)
    {
        pimpl_->text        = std::move(text);
        pimpl_->hasErrors   = hasErrors;
    }
    else
        pimpl_ = new Report::Pimpl{ std::move(text), hasErrors };
}

void Report::Printf(const char* format, ...)
{
    /* Reset report with error flags disabled */
    if (pimpl_ == nullptr)
        pimpl_ = new Report::Pimpl{};

    /* Forward formatted string with variadic arguments to internal function */
    LLGL_STRING_PRINTF(pimpl_->text, format);
}

void Report::Errorf(const char* format, ...)
{
    /* Reset report with error flags enabled */
    if (pimpl_ == nullptr)
        pimpl_ = new Report::Pimpl{};

    pimpl_->hasErrors = true;

    /* Forward formatted string with variadic arguments to internal function */
    LLGL_STRING_PRINTF(pimpl_->text, format);
}

Report& Report::operator = (const Report& rhs)
{
    if (this != &rhs)
    {
        if (rhs.pimpl_ == nullptr)
        {
            delete pimpl_;
            pimpl_ = nullptr;
        }
        else
        {
            if (pimpl_ == nullptr)
                pimpl_ = new Report::Pimpl{};
            *pimpl_ = *rhs.pimpl_;
        }
    }
    return *this;
}

Report& Report::operator = (Report&& rhs)
{
    if (this != &rhs)
    {
        delete pimpl_;
        pimpl_ = rhs.pimpl_;
        rhs.pimpl_ = nullptr;
    }
    return *this;
}

Report::operator bool () const
{
    return (pimpl_ != nullptr && (!pimpl_->text.empty() || pimpl_->hasErrors));
}


} // /namespace LLGL



// ================================================================================
