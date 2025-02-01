/*
 * DXShaderReflection.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DXShaderReflection.h"
#include <d3dcommon.h>


namespace LLGL
{


ShaderResourceReflection* FetchOrInsertResource(
    ShaderReflection&   outReflection,
    const char*         name,
    const ResourceType  type,
    std::uint32_t       slot)
{
    /* Fetch resource from list */
    for (ShaderResourceReflection& resource : outReflection.resources)
    {
        if (resource.binding.type == type &&
            resource.binding.slot == slot &&
            resource.binding.name.compare(name) == 0)
        {
            return (&resource);
        }
    }

    /* Allocate new resource and initialize parameters */
    outReflection.resources.resize(outReflection.resources.size() + 1);
    ShaderResourceReflection* ref = &(outReflection.resources.back());
    {
        ref->binding.name = std::string(name);
        ref->binding.type = type;
        ref->binding.slot = slot;
    }
    return ref;
}

//TODO: merge with LLGL::MakeUniformVectorType()
UniformType DXMakeUniformVectorType(UniformType baseType, UINT elements)
{
    if (elements >= 1 && elements <= 4)
        return static_cast<UniformType>(static_cast<int>(baseType) + (elements - 1));
    else
        return UniformType::Undefined;
}

//TODO: merge with LLGL::MakeUniformMatrixType()
UniformType DXMakeUniformMatrixType(UniformType baseType, UINT rows, UINT cols)
{
    if (rows < 2 || cols < 2)
    {
        if (baseType == UniformType::Float2x2)
            return DXMakeUniformVectorType(UniformType::Float1, std::max<int>(rows, cols));
    }
    else if (rows <= 4 && cols <= 4)
        return static_cast<UniformType>(static_cast<int>(baseType) + (rows - 2)*3 + (cols - 2));
    return UniformType::Undefined;
}

UniformType DXMapD3DShaderScalarTypeToUniformType(D3D_SHADER_VARIABLE_TYPE type)
{
    switch (type)
    {
        case D3D_SVT_BOOL:  return UniformType::Bool1;
        case D3D_SVT_FLOAT: return UniformType::Float1;
        case D3D_SVT_INT:   return UniformType::Int1;
        case D3D_SVT_UINT:  return UniformType::UInt1;
        default:            return UniformType::Undefined;
    }
}

UniformType DXMapD3DShaderVectorTypeToUniformType(D3D_SHADER_VARIABLE_TYPE type, UINT elements)
{
    switch (type)
    {
        case D3D_SVT_BOOL:  return DXMakeUniformVectorType(UniformType::Bool1, elements);
        case D3D_SVT_FLOAT: return DXMakeUniformVectorType(UniformType::Float1, elements);
        case D3D_SVT_INT:   return DXMakeUniformVectorType(UniformType::Int1, elements);
        case D3D_SVT_UINT:  return DXMakeUniformVectorType(UniformType::UInt1, elements);
        default:            return UniformType::Undefined;
    }
}

UniformType DXMapD3DShaderMatrixTypeToUniformType(D3D_SHADER_VARIABLE_TYPE type, UINT rows, UINT cols)
{
    switch (type)
    {
        case D3D_SVT_FLOAT: return DXMakeUniformMatrixType(UniformType::Float2x2, rows, cols);
        default:            return UniformType::Undefined;
    }
}


} // /namespace LLGL



// ================================================================================
