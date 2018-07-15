/*
 * DescriptorHelper.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DESCRIPTOR_HELPER_H
#define LLGL_DESCRIPTOR_HELPER_H


#include <LLGL/Export.h>
#include <LLGL/ForwardDecls.h>
#include <cstdint>


namespace LLGL
{


/* ----- Functions ----- */

// Fills the buffer with the indices for the color attachments that are meant to be cleared.
LLGL_EXPORT std::uint8_t FillClearColorAttachmentIndices(std::uint8_t* clearColorAttachments, const RenderPassDescriptor& renderPassDesc);


} // /namespace LLGL


#endif



// ================================================================================
