/*
 * RenderTargetUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_TARGET_UTILS_H
#define LLGL_RENDER_TARGET_UTILS_H


#include <LLGL/Texture.h>
#include <LLGL/RenderTargetFlags.h>


namespace LLGL
{


/* ----- Functions ----- */

// Returns true if the specified render-target attachment is enabled, i.e. either 'texture' or 'format' is valid.
inline bool IsAttachmentEnabled(const AttachmentDescriptor& attachmentDesc)
{
    return (attachmentDesc.texture != nullptr || attachmentDesc.format != Format::Undefined);
}

// Returns the format of the specified render-target attachment.
inline Format GetAttachmentFormat(const AttachmentDescriptor& desc)
{
    if (desc.format != Format::Undefined)
        return desc.format;
    if (auto texture = desc.texture)
        return texture->GetFormat();
    return Format::Undefined;
}


} // /namespace LLGL


#endif



// ================================================================================
