/*
 * WGIndexBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_INDEX_BUFFER_H
#define LLGL_WG_INDEX_BUFFER_H


#include "WGBuffer.h"


namespace LLGL
{


class WGIndexBuffer final : public WGBuffer
{

    public:

        WGIndexBuffer(WGPUDevice device, const BufferDescriptor& bufferDesc);

        // Returns the native WebGPU index format.
        inline WGPUIndexFormat GetWGIndexFormat() const
        {
            return indexFormat_;
        }

    private:

        WGPUIndexFormat indexFormat_ = WGPUIndexFormat_Undefined;

};


} // /namespace LLGL


#endif



// ================================================================================
