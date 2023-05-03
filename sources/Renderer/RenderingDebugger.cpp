/*
 * RenderingDebugger.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderingDebugger.h>
#include <LLGL/Log.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Container/Strings.h>
#include <map>


namespace LLGL
{


struct CompareStringLess
{
    inline bool operator () (const UTF8String& lhs, const UTF8String& rhs) const
    {
        return (lhs.compare(rhs) < 0);
    }
    inline bool operator () (const UTF8String& lhs, const StringView& rhs) const
    {
        return (lhs.compare(rhs) < 0);
    }
    inline bool operator () (const StringView& lhs, const UTF8String& rhs) const
    {
        return (lhs.compare(rhs) < 0);
    }
};

template <typename T>
using UTF8StringMap = std::map<UTF8String, T, CompareStringLess>;

struct RenderingDebugger::Pimpl
{
    UTF8StringMap<Message>  errors;
    UTF8StringMap<Message>  warnings;
    const char*             source      = "";
    const char*             groupName   = "";
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
    UTF8String str = message.ToReportString();
    Log::Errorf("error (%s): %s\n", ToString(type), str.c_str());
    message.Block();
}

void RenderingDebugger::OnWarning(WarningType type, Message& message)
{
    UTF8String str = message.ToReportString();
    Log::Printf("warning (%s): %s\n", ToString(type), str.c_str());
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

UTF8String RenderingDebugger::Message::ToReportString() const
{
    UTF8String s;

    if (!GetSource().empty())
    {
        s += "in '";
        s += GetSource();
        s += "'";
    }

    if (!GetGroupName().empty())
    {
        if (!s.empty())
            s += " ";
        s += "during '";
        s += GetGroupName();
        s += "'";
    }

    if (!s.empty())
        s += ": ";

    s += GetText();

    return s;
}

void RenderingDebugger::Message::IncOccurrence()
{
    ++occurrences_;
}


} // /namespace LLGL



// ================================================================================
