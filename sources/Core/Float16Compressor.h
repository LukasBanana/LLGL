/*
 * Float16Compressor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_FLOAT16_COMPRESSOR_H
#define LLGL_FLOAT16_COMPRESSOR_H


#include <LLGL/Export.h>
#include <cstdint>


namespace LLGL
{


// Compresses the specified 32-bit float into a 16-bit float (represented as 16-bit unsigned integer).
LLGL_EXPORT std::uint16_t CompressFloat16(float value);

// Decompresses the specified 16-bit float (represented as 16-bit unsigned integer) into a 32-bit float.
LLGL_EXPORT float DecompressFloat16(std::uint16_t value);


} // /namespace LLGL


#endif



// ================================================================================
