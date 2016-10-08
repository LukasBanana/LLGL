/*
 * RenderingDebugger.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderingDebugger.h>


namespace LLGL
{


RenderingDebugger::~RenderingDebugger()
{
}

void RenderingDebugger::PostError(ErrorType type, const std::string& message, const std::string& source)
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
        errors_[message] = Message(message, source);
        OnError(type, errors_[message]);
    }
}

void RenderingDebugger::PostWarning(WarningType type, const std::string& message, const std::string& source)
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
        warnings_[message] = Message(message, source);
        OnWarning(type, warnings_[message]);
    }
}


/*
 * ====== Protected: =======
 */

void RenderingDebugger::OnError(ErrorType type, Message& message)
{
    // dummy
}
        
void RenderingDebugger::OnWarning(WarningType type, Message& message)
{
    // dummy
}


/*
 * Message class
 */

RenderingDebugger::Message::Message(const std::string& text, const std::string& source) :
    text_   ( text   ),
    source_ ( source )
{
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

void RenderingDebugger::Message::IncOccurrence()
{
    ++occurrences_;
}


} // /namespace LLGL



// ================================================================================
