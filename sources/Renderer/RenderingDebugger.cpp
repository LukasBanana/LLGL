/*
 * RenderingDebugger.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderingDebugger.h>
#include <LLGL/Log.h>
#include <LLGL/Misc/TypeNames.h>
#include <LLGL/Container/Strings.h>
#include <map>


namespace LLGL
{


struct RenderingDebugger::Pimpl
{
    std::map<UTF8String, Message>   errors;
    std::map<UTF8String, Message>   warnings;
    const char*                     source      = "";
    const char*                     groupName   = "";
};


RenderingDebugger::RenderingDebugger() :
    pimpl_ { new Pimpl{} }
{
}

RenderingDebugger::~RenderingDebugger()
{
    delete pimpl_;
}

void RenderingDebugger::SetSource(const char* source)
{
    pimpl_->source = (source != nullptr ? source : "");
}

void RenderingDebugger::SetDebugGroup(const char* name)
{
    pimpl_->groupName = (name != nullptr ? name : "");
}

void RenderingDebugger::PostError(const ErrorType type, const StringView& message)
{
    auto it = pimpl_->errors.find(message);
    if (it != pimpl_->errors.end())
    {
        if (!it->second.IsBlocked())
        {
            it->second.IncOccurrence();
            OnError(type, it->second);
        }
    }
    else
    {
        auto& msg = pimpl_->errors[message];
        msg = Message{ message, pimpl_->source, pimpl_->groupName };
        OnError(type, msg);
    }
}

void RenderingDebugger::PostWarning(const WarningType type, const StringView& message)
{
    auto it = pimpl_->warnings.find(message);
    if (it != pimpl_->warnings.end())
    {
        if (!it->second.IsBlocked())
        {
            it->second.IncOccurrence();
            OnWarning(type, it->second);
        }
    }
    else
    {
        auto& msg = pimpl_->warnings[message];
        msg = Message{ message, pimpl_->source, pimpl_->groupName };
        OnWarning(type, msg);
    }
}


/*
 * ====== Protected: =======
 */

void RenderingDebugger::OnError(ErrorType type, Message& message)
{
    Log::PostReport(
        Log::ReportType::Error,
        message.ToReportString("ERROR (" + UTF8String(ToString(type)) + ")")
    );
    message.Block();
}

void RenderingDebugger::OnWarning(WarningType type, Message& message)
{
    Log::PostReport(
        Log::ReportType::Warning,
        message.ToReportString("WARNING (" + UTF8String(ToString(type)) + ")")
    );
    message.Block();
}


/*
 * Message class
 */

RenderingDebugger::Message::Message(const StringView& text, const StringView& source, const StringView& groupName) :
    text_      { text      },
    source_    { source    },
    groupName_ { groupName }
{
    /* Replace "LLGL::Dbg" by "LLGL::" */
    //if (source_.compare(0, 9, "LLGL::Dbg") == 0)
    //    source_ = "LLGL::" + source_.substr(9);
}

void RenderingDebugger::Message::Block()
{
    blocked_ = true;
}

void RenderingDebugger::Message::BlockAfter(std::size_t occurrences)
{
    if (GetOccurrences() >= occurrences)
        Block();
}

UTF8String RenderingDebugger::Message::ToReportString(const StringView& reportTypeName) const
{
    UTF8String s;

    s += reportTypeName;
    s += ": ";

    if (!GetGroupName().empty())
    {
        s += "during '";
        s += GetGroupName();
        s += "': ";
    }

    if (!GetSource().empty())
    {
        s += "in '";
        s += GetSource();
        s += "': ";
    }

    s += GetText();

    return s;
}

void RenderingDebugger::Message::IncOccurrence()
{
    ++occurrences_;
}


} // /namespace LLGL



// ================================================================================
