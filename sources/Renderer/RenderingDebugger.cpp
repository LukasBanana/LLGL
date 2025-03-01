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
#include "../Core/StringUtils.h"
#include "../Platform/Debug.h"
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
    FrameProfile            frameProfile;
    const char*             source                  = "";
    const char*             groupName               = "";
    bool                    isTimeRecording         = false;
    bool                    isBreakOnErrorEnabled   = false;
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

void RenderingDebugger::SetTimeRecording(bool enabled)
{
    pimpl_->isTimeRecording = enabled;
}

bool RenderingDebugger::GetTimeRecording() const
{
    return pimpl_->isTimeRecording;
}

void RenderingDebugger::SetBreakOnError(bool enable)
{
    pimpl_->isBreakOnErrorEnabled = enable;
}

bool RenderingDebugger::GetBreakOnError() const
{
    return pimpl_->isBreakOnErrorEnabled;
}

void RenderingDebugger::Errorf(const ErrorType type, const char* format, ...)
{
    /* Print formatted string */
    std::string message;
    LLGL_STRING_PRINTF(message, format);

    /* Check if there is already an entry for the exact same message */
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
        /* Allocate new error entry */
        Message& msg = pimpl_->errors[message];
        msg = Message{ message, pimpl_->source, pimpl_->groupName };
        OnError(type, msg);
    }
}

void RenderingDebugger::Warningf(const WarningType type, const char* format, ...)
{
    /* Print formatted string */
    std::string message;
    LLGL_STRING_PRINTF(message, format);

    /* Check if there is already an entry for the exact same message */
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
        /* Allocate new warning entry */
        Message& msg = pimpl_->warnings[message];
        msg = Message{ message, pimpl_->source, pimpl_->groupName };
        OnWarning(type, msg);
    }
}

void RenderingDebugger::FlushProfile(FrameProfile* outputProfile)
{
    /* Copy current counters to the output profile (if set) */
    if (outputProfile)
        *outputProfile = std::move(pimpl_->frameProfile);

    /* Clear values */
    pimpl_->frameProfile = {};
}

void RenderingDebugger::RecordProfile(const FrameProfile& profile)
{
    RenderingDebugger::MergeProfiles(pimpl_->frameProfile, profile);
}

void RenderingDebugger::PostError(const ErrorType type, const StringView& message)
{
    const std::string str(message.begin(), message.end());
    Errorf(type, "%s", str.c_str());
}

void RenderingDebugger::PostWarning(const WarningType type, const StringView& message)
{
    const std::string str(message.begin(), message.end());
    Warningf(type, "%s", str.c_str());
}

#define LLGL_ASSERT_STRUCT_FIELDS(TYPE, FIELDS) \
    static_assert(sizeof(TYPE) == alignof(TYPE)*(FIELDS), "unexpected number of fields in struct 'LLGL::" #TYPE "'");

static void MergeProfileCommandQueueRecords(ProfileCommandQueueRecord& dst, const ProfileCommandQueueRecord& src)
{
    LLGL_ASSERT_STRUCT_FIELDS(ProfileCommandQueueRecord, 7);
    dst.bufferWrites                += src.bufferWrites             ;
    dst.bufferReads                 += src.bufferReads              ;
    dst.bufferMappings              += src.bufferMappings           ;
    dst.textureWrites               += src.textureWrites            ;
    dst.textureReads                += src.textureReads             ;
    dst.commandBufferSubmittions    += src.commandBufferSubmittions ;
    dst.fenceSubmissions            += src.fenceSubmissions         ;
}

static void MergeProfileCommandBufferRecords(ProfileCommandBufferRecord& dst, const ProfileCommandBufferRecord& src)
{
    LLGL_ASSERT_STRUCT_FIELDS(ProfileCommandBufferRecord, 24);
    dst.encodings                   += src.encodings                ;
    dst.mipMapsGenerations          += src.mipMapsGenerations       ;
    dst.vertexBufferBindings        += src.vertexBufferBindings     ;
    dst.indexBufferBindings         += src.indexBufferBindings      ;
    dst.constantBufferBindings      += src.constantBufferBindings   ;
    dst.sampledBufferBindings       += src.sampledBufferBindings    ;
    dst.storageBufferBindings       += src.storageBufferBindings    ;
    dst.sampledTextureBindings      += src.sampledTextureBindings   ;
    dst.storageTextureBindings      += src.storageTextureBindings   ;
    dst.samplerBindings             += src.samplerBindings          ;
    dst.resourceHeapBindings        += src.resourceHeapBindings     ;
    dst.graphicsPipelineBindings    += src.graphicsPipelineBindings ;
    dst.computePipelineBindings     += src.computePipelineBindings  ;
    dst.attachmentClears            += src.attachmentClears         ;
    dst.bufferUpdates               += src.bufferUpdates            ;
    dst.bufferCopies                += src.bufferCopies             ;
    dst.bufferFills                 += src.bufferFills              ;
    dst.textureCopies               += src.textureCopies            ;
    dst.renderPassSections          += src.renderPassSections       ;
    dst.streamOutputSections        += src.streamOutputSections     ;
    dst.querySections               += src.querySections            ;
    dst.renderConditionSections     += src.renderConditionSections  ;
    dst.drawCommands                += src.drawCommands             ;
    dst.dispatchCommands            += src.dispatchCommands         ;
}

void RenderingDebugger::MergeProfiles(FrameProfile& dst, const FrameProfile& src)
{
    /* Accumulate counters */
    MergeProfileCommandQueueRecords(dst.commandQueueRecord, src.commandQueueRecord);
    MergeProfileCommandBufferRecords(dst.commandBufferRecord, src.commandBufferRecord);

    /* Append time records */
    dst.timeRecords.insert(dst.timeRecords.end(), src.timeRecords.begin(), src.timeRecords.end());
}


/*
 * ====== Protected: =======
 */

void RenderingDebugger::OnError(ErrorType type, Message& message)
{
    UTF8String str = message.ToReportString();
    Log::Errorf(Log::ColorFlags::StdError, "error");
    Log::Errorf(" (%s): %s\n", ToString(type), str.c_str());
    message.Block();

    if (GetBreakOnError())
        DebugBreakOnError();
}

void RenderingDebugger::OnWarning(WarningType type, Message& message)
{
    UTF8String str = message.ToReportString();
    Log::Printf(Log::ColorFlags::StdWarning, "warning");
    Log::Printf(" (%s): %s\n", ToString(type), str.c_str());
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
