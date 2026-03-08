/*
 * D3D9ConstantTableParser.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9ConstantTableParser.h"
#include "D3D9Shader.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/ArrayView.h>
#include <d3d9.h>


namespace LLGL
{


// Mimics D3DXSHADER_CONSTANTTABLE
struct D3DXShaderConstantTable
{
    DWORD   size;          // Size of this struct
    DWORD   creator;       // Offset to string
    DWORD   version;       // Shader version
    DWORD   constants;     // Number of constants
    DWORD   constantInfo;  // Offset to array of D3DXShaderConstantInfo
    DWORD   flags;
    DWORD   target;        // Offset to profile string
};

// Mimics D3DXSHADER_CONSTANTINFO
struct D3DXShaderConstantInfo
{
    DWORD   name;          // Offset to string
    WORD    registerSet;   // BOO, INT4, FLOAT4, SAMPLER etc.
    WORD    registerIndex;
    WORD    registerCount;
    WORD    reserved;
    DWORD   typeInfo;      // Offset to D3DXShaderTypeInfo
    DWORD   defaultValue;  // Optional offset to data
};

// Mimics D3DXSHADER_TYPEINFO
struct D3DXShaderTypeInfo
{
    WORD    classType;         // Scalar, vector, matrix, struct
    WORD    baseType;          // Float, int, bool, sampler etc.
    WORD    rows;
    WORD    columns;
    WORD    elements;          // Array size
    WORD    structMembers;     // Number of struct members
    DWORD   structMemberInfo;  // Offset to members
};

// Mimics D3DXSHADER_STRUCTMEMBERINFO
struct D3DXShaderStructMemberInfo
{
    DWORD   name;       // Offset to string
    DWORD   typeInfo;   // Offset to D3DXShaderTypeInfo
};

// Mimics entries of interest for D3DXPARAMETER_TYPE
enum D3DXParameterType
{
    D3DXParameterType_Void,
    D3DXParameterType_Bool,
    D3DXParameterType_Int,
    D3DXParameterType_Float,
};

static D3D9UniformType ToD3D9UniformType(D3DXParameterType componentType)
{
    switch (componentType)
    {
        case D3DXParameterType_Bool:    return D3D9UniformType::Bool;
        case D3DXParameterType_Int:     return D3D9UniformType::Int;
        case D3DXParameterType_Float:   return D3D9UniformType::Float;
        default:                        return D3D9UniformType::Undefined;
    }
}

static void ParseSM3Constant(const BYTE* base, D3D9ShaderConstant& constant, DWORD nameOffset, DWORD typeInfoOffset)
{
    const char* name = reinterpret_cast<const char*>(base + nameOffset);
    const auto* type = reinterpret_cast<const D3DXShaderTypeInfo*>(base + typeInfoOffset);

    constant.name       = name;
    constant.type       = ToD3D9UniformType(static_cast<D3DXParameterType>(type->baseType));
    constant.rows       = type->rows;
    constant.columns    = type->columns;
    constant.arraySize  = type->elements;

    const ArrayView<D3DXShaderStructMemberInfo> structMembers
    {
        reinterpret_cast<const D3DXShaderStructMemberInfo*>(base + type->structMemberInfo),
        type->structMembers
    };

    constant.structMembers.resize(type->structMembers);
    for_range(i, type->structMembers)
        ParseSM3Constant(base, constant.structMembers[i], structMembers[i].name, structMembers[i].typeInfo);
}

static void CalculateRegisterSlotsAndByteSizes(D3D9ShaderConstant& constant, D3D9ShaderRegister& regBase, UINT& byteOffset)
{
    if (constant.type == D3D9UniformType::Undefined)
    {
        for_range(i, constant.structMembers.size())
            CalculateRegisterSlotsAndByteSizes(constant.structMembers[i], regBase, constant.byteSize);
    }
    else
    {
        UINT paddingOffset  = 0;
        byteOffset += paddingOffset;

        constant.byteSize;
    }
    byteOffset += constant.byteSize;
}

HRESULT D3DParseSM3ConstantTable(const void* byteCode, D3D9ShaderConstantTable& outCtable)
{
    for (const DWORD* ptr = static_cast<const DWORD*>(byteCode); *ptr != D3DSIO_END;)
    {
        DWORD token = *ptr++;
        if ((token & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
        {
            UINT size = (token >> 16) & 0xFFFF;
            DWORD fourCC = *ptr++;
            if (fourCC == *reinterpret_cast<const UINT*>("CTAB"))
            {
                const BYTE* base = reinterpret_cast<const BYTE*>(ptr);
                auto* table = reinterpret_cast<const D3DXShaderConstantTable*>(base);
                if (table->size != sizeof(D3DXShaderConstantTable))
                    return E_INVALIDARG;

                const ArrayView<D3DXShaderConstantInfo> constants
                {
                    reinterpret_cast<const D3DXShaderConstantInfo*>(base + table->constantInfo),
                    table->constants
                };

                outCtable.constants.resize(table->constants);
                for_range(i, table->constants)
                {
                    /* Parse all constant information */
                    const D3DXShaderConstantInfo& cinfo = constants[i];
                    {
                        outCtable.constants[i].reg.index = cinfo.registerIndex;
                        outCtable.constants[i].reg.count = cinfo.registerCount;
                    }
                    ParseSM3Constant(base, outCtable.constants[i], cinfo.name, cinfo.typeInfo);

                    /* Calculate register index, register count, and bytesize for each constant's struct members */
                    D3D9ShaderRegister regBase = outCtable.constants[i].reg;
                    CalculateRegisterSlotsAndByteSizes(outCtable.constants[i], regBase, outCtable.byteSize);
                }

                return S_OK;
            }
        }
    }
    return E_FAIL;
}


} // /namespace LLGL



// ================================================================================
