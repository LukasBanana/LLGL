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
#include <type_traits>
#include "../Direct3D9.h"
#include "../../StaticAssertions.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


// Mimics entries of interest for D3DXPARAMETER_TYPE
enum D3DXParameterType : WORD
{
    D3DXParameterType_Void,
    D3DXParameterType_Bool,
    D3DXParameterType_Int,
    D3DXParameterType_Float,
    D3DXParameterType_String,
    D3DXParameterType_Texture,
    D3DXParameterType_Texture1D,
    D3DXParameterType_Texture2D,
    D3DXParameterType_Texture3D,
    D3DXParameterType_TextureCube,
    D3DXParameterType_Sampler,
    D3DXParameterType_Sampler1D,
    D3DXParameterType_Sampler2D,
    D3DXParameterType_Sampler3D,
    D3DXParameterType_SamplerCUBE,
    D3DXParameterType_PixelShader,
    D3DXParameterType_VertexShader,
    D3DXParameterType_PixelFragment,
    D3DXParameterType_VertexFragment,
    D3DXParameterType_Unsupported,
};

enum D3DXRegisterSet : WORD
{
    D3DXRegisterSet_Bool,
    D3DXRegisterSet_Int4,
    D3DXRegisterSet_Float4,
    D3DXRegisterSet_Sampler,
};

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

LLGL_ASSERT_POD_TYPE(D3DXShaderConstantTable);

// Mimics D3DXSHADER_CONSTANTINFO
struct D3DXShaderConstantInfo
{
    DWORD           name;          // Offset to string
    D3DXRegisterSet registerSet;   // BOO, INT4, FLOAT4, SAMPLER etc.
    WORD            registerIndex;
    WORD            registerCount;
    WORD            reserved;
    DWORD           typeInfo;      // Offset to D3DXShaderTypeInfo
    DWORD           defaultValue;  // Offset from the beginning of the struct to the string that contains the default value
};

LLGL_ASSERT_POD_TYPE(D3DXShaderConstantInfo);

// Mimics D3DXSHADER_TYPEINFO
struct D3DXShaderTypeInfo
{
    WORD                classType;         // Scalar, vector, matrix, struct
    D3DXParameterType   baseType;          // Float, int, bool, sampler etc.
    WORD                rows;
    WORD                columns;
    WORD                elements;          // Array size
    WORD                structMembers;     // Number of struct members
    DWORD               structMemberInfo;  // Offset to members
};

LLGL_ASSERT_POD_TYPE(D3DXShaderTypeInfo);

// Mimics D3DXSHADER_STRUCTMEMBERINFO
struct D3DXShaderStructMemberInfo
{
    DWORD   name;       // Offset to string
    DWORD   typeInfo;   // Offset to D3DXShaderTypeInfo
};

LLGL_ASSERT_POD_TYPE(D3DXShaderStructMemberInfo);

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

static D3D9UniformType ToD3D9UniformType(D3DXRegisterSet registerSet)
{
    switch (registerSet)
    {
        case D3DXRegisterSet_Bool:      return D3D9UniformType::Bool;
        case D3DXRegisterSet_Int4:      return D3D9UniformType::Int;
        case D3DXRegisterSet_Float4:    return D3D9UniformType::Float;
        default:                        return D3D9UniformType::Undefined;
    }
}

static void ParseSM3Constant(const BYTE* base, D3D9ShaderConstant& constant, DWORD nameOffset, DWORD typeInfoOffset)
{
    const char* name = reinterpret_cast<const char*>(base + nameOffset);
    const auto* type = reinterpret_cast<const D3DXShaderTypeInfo*>(base + typeInfoOffset);

    constant.name       = name;
    //constant.type       = ToD3D9UniformType(type->baseType);
    constant.rows       = type->rows;
    constant.columns    = type->columns;
    constant.arraySize  = type->elements;

#if 0
    const ArrayView<D3DXShaderStructMemberInfo> structMembers
    {
        reinterpret_cast<const D3DXShaderStructMemberInfo*>(base + type->structMemberInfo),
        type->structMembers
    };

    constant.structMembers.resize(type->structMembers);
    for_range(i, type->structMembers)
        ParseSM3Constant(base, constant.structMembers[i], structMembers[i].name, structMembers[i].typeInfo);
#endif
}

constexpr std::uint32_t k_d3dComponentSize = sizeof(float);
constexpr std::uint32_t k_d3dRegisterSize = 4u * k_d3dComponentSize;

static std::uint32_t GetLowerRegisterOffset(std::uint32_t offset)
{
    return (offset - offset % k_d3dRegisterSize);
};

#if 0
// Returns the total size of the specified constant.
static void CalculateRegisterSlotsAndByteSizes(D3D9ShaderConstant& constant, D3D9ShaderRegister& regBase, std::uint32_t& byteOffset)
{
    constant.byteOffset = byteOffset;

    if (constant.type == D3D9UniformType::Undefined)
    {
        if (!constant.structMembers.empty())
        {
            for_range(i, constant.structMembers.size())
                CalculateRegisterSlotsAndByteSizes(constant.structMembers[i], regBase, byteOffset);

            constant.byteSize = (byteOffset - constant.byteOffset);
        }
    }
    else
    {
        LLGL_ASSERT(constant.rows > 0);
        LLGL_ASSERT(constant.columns > 0);
        constant.byteSize = (constant.rows - 1) * k_d3dRegisterSize + (constant.columns) * k_d3dComponentSize;

        const std::uint32_t startRegisterOffset = GetLowerRegisterOffset(byteOffset);
        const std::uint32_t targetOffset = startRegisterOffset + constant.reg.component * k_d3dComponentSize;

        if (constant.byteOffset <= targetOffset && startRegisterOffset == GetLowerRegisterOffset(targetOffset + constant.byteSize))
        {
            /* Use target offset if we did not cross register boundaries, e.g. "float2 A; float2 B : register(c0.zw);" both fall into register 0 */
            constant.byteOffset = targetOffset;
        }
        else
        {
            /* Add padding to move offset into next register */
            const std::uint32_t paddingOffset = GetAlignedSize<std::uint32_t>(byteOffset, k_d3dRegisterSize) % k_d3dRegisterSize;
            constant.byteOffset += paddingOffset;
        }

        /* Update output byte offset */
        byteOffset = constant.byteOffset + constant.byteSize;
    }

    /* Assign register slots */
    constant.reg.index = GetLowerRegisterOffset(constant.byteOffset) / k_d3dRegisterSize;
    constant.reg.count = DivideRoundUp(constant.byteSize, k_d3dRegisterSize);
}
#endif

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
                    D3D9ShaderConstant& constant = outCtable.constants[i];

                    /* Parse all constant information */
                    const D3DXShaderConstantInfo& cinfo = constants[i];
                    {
                        constant.type       = ToD3D9UniformType(cinfo.registerSet);
                        constant.reg.index  = cinfo.registerIndex;
                        constant.reg.count  = cinfo.registerCount;
                    }
                    ParseSM3Constant(base, constant, cinfo.name, cinfo.typeInfo);

#if 0
                    /* Calculate register index, register count, and bytesize for each constant's struct members */
                    D3D9ShaderRegister regBase = constant.reg;
                    CalculateRegisterSlotsAndByteSizes(constant, regBase, outCtable.byteSize);
#else
                    /* Determine byte offset and size by register index and count */
                    constant.byteOffset = constant.reg.index * k_d3dRegisterSize;
                    constant.byteSize   = constant.reg.count * k_d3dRegisterSize;
#endif
                }

                return S_OK;
            }
        }
    }
    return E_FAIL;
}


} // /namespace LLGL



// ================================================================================
