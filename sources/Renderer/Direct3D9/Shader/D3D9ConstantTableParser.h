/*
 * D3D9ConstantTableParser.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_CONSTANT_TABLE_PARSER_H
#define LLGL_D3D9_CONSTANT_TABLE_PARSER_H


#include <d3dcommon.h>


namespace LLGL
{


struct D3D9ShaderConstantTable;

// Extracts the shader constant table from the specified SM2 & SM3 bytecode blob.
HRESULT D3DParseSM3ConstantTable(const void* byteCode, D3D9ShaderConstantTable& outCtable);


} // /namespace LLGL


#endif



// ================================================================================
