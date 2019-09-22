/*
 * RenderingDebugger.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderingDebugger.h>
#include <LLGL/Strings.h>
#include <LLGL/Log.h>


namespace LLGL
{


void RenderingDebugger::SetSource(const char* source)
{
    source_ = (source != nullptr ? source : "");
}

void RenderingDebugger::SetDebugGroup(const char* name)
{
    groupName_ = (name != nullptr ? name : "");
}

void RenderingDebugger::PostError(const ErrorType type, const std::string& message)
{
    auto it = errors_.find(message);
    if (it != errors_.end())
    {
        if (!it->second.IsBlocked())
        {
            it->second.IncOccurrence();
            OnError(type, it->second);
        }
    }
    else
    {
        errors_[message] = Message{ message, source_, groupName_ };
        OnError(type, errors_[message]);
    }
}

void RenderingDebugger::PostWarning(const WarningType type, const std::string& message)
{
    auto it = warnings_.find(message);
    if (it != warnings_.end())
    {
        if (!it->second.IsBlocked())
        {
            it->second.IncOccurrence();
            OnWarning(type, it->second);
        }
    }
    else
    {
        warnings_[message] = Message{ message, source_, groupName_ };
        OnWarning(type, warnings_[message]);
    }
}


/*
 * ====== Protected: =======
 */

void RenderingDebugger::OnError(ErrorType type, Message& message)
{
    Log::PostReport(
        Log::ReportType::Error,
        message.ToReportString("ERROR (" + std::string(ToString(type)) + ')')
    );
    message.Block();
}

void RenderingDebugger::OnWarning(WarningType type, Message& message)
{
    Log::PostReport(
        Log::ReportType::Warning,
        message.ToReportString("WARNING (" + std::string(ToString(type)) + ')')
    );
    message.Block();
}


/*
 * Message class
 */

RenderingDebugger::Message::Message(const std::string& text, const std::string& source, const std::string& groupName) :
    text_      { text      },
    source_    { source    },
    groupName_ { groupName }
{
    /* Replace "LLGL::Dbg" by "LLGL::" */
    if (source_.compare(0, 9, "LLGL::Dbg") == 0)
        source_ = "LLGL::" + source_.substr(9);
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

std::string RenderingDebugger::Message::ToReportString(const std::string& reportTypeName) const
{
    std::string s;
    
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
