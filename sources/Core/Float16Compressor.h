/*
 * Float16Compressor.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
