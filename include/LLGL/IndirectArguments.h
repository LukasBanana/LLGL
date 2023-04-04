/*
 * IndirectArguments.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_INDIRECT_ARGUMENTS_H
#define LLGL_INDIRECT_ARGUMENTS_H


#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\defgroup group_indirect_args Buffer argument structures for indirect draw and compute commands.
\addtogroup group_indirect_args
@{
*/

/**
\brief Format structure for the arguments of an indirect draw command.
\remarks This structure is byte aligned, i.e. it can be reinterpret casted to a buffer in CPU memory space.
\note This is a plain-old-data (POD) structure, so it has no default constructor to make it easily compatible with the GPU memory space.
\see CommandBuffer::DrawIndirect
\see OpenGL counterpart \c DrawArraysIndirectCommand: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_draw_indirect.txt
\see Vulkan counterpart \c VkDrawIndirectCommand: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDrawIndirectCommand.html
\see Direct3D11 counterpart \c D3D11_DRAW_INSTANCED_INDIRECT_ARGS: https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ns-d3d11-d3d11_draw_instanced_indirect_args
\see Direct3D12 counterpart \c D3D12_DRAW_ARGUMENTS: https://docs.microsoft.com/en-us/windows/desktop/api/D3D12/ns-d3d12-d3d12_draw_arguments
\see Metal counterpart \c MTLDrawPrimitivesIndirectArguments: https://developer.apple.com/documentation/metal/mtldrawprimitivesindirectarguments?language=objc
*/
struct DrawIndirectArguments
{
    //! Specifies the number of vertices per instance.
    std::uint32_t   numVertices;

    //! Specifies the number of instances to draw.
    std::uint32_t   numInstances;

    /**
    \brief Specifies the zero-based offset of the first vertex from the vertex buffer.
    \note This parameter modifies the vertex ID within the shader pipeline differently for \c SV_VertexID
    in HLSL and \c gl_VertexID in GLSL (or \c gl_VertexIndex for Vulkan), due to rendering API differences.
    The system value \c SV_VertexID in HLSL will always start with zero,
    but the system value \c gl_VertexID in GLSL (or \c gl_VertexIndex for Vulkan)
    will start with the value of \c firstVertex.
    */
    std::uint32_t   firstVertex;

    /**
    \brief Specifies the zero-based offset of the first instance.
    \note This parameter modifies the instance ID within the shader pipeline differently for \c SV_InstanceID
    in HLSL and \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan), due to rendering API differences.
    The system value \c SV_InstanceID in HLSL will always start with zero,
    but the system value \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan)
    will start with the value of \c firstInstance.
    */
    std::uint32_t   firstInstance;
};

/**
\brief Format structure for the arguments of an indirect indexed draw command.
\remarks This structure is byte aligned, i.e. it can be reinterpret casted to a buffer in CPU memory space.
\note This is a plain-old-data (POD) structure, so it has no default constructor to make it easily compatible with the GPU memory space.
\see CommandBuffer::DrawIndexedIndirect
\see OpenGL counterpart \c DrawElementsIndirectCommand: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_draw_indirect.txt
\see Vulkan counterpart \c VkDrawIndexedIndirectCommand: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDrawIndexedIndirectCommand.html
\see Direct3D11 counterpart \c D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS: https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ns-d3d11-d3d11_draw_indexed_instanced_indirect_args
\see Direct3D12 counterpart \c D3D12_DRAW_INDEXED_ARGUMENTS: https://docs.microsoft.com/en-us/windows/desktop/api/D3D12/ns-d3d12-d3d12_draw_indexed_arguments
\see Metal counterpart \c MTLDrawIndexedPrimitivesIndirectArguments: https://developer.apple.com/documentation/metal/mtldrawindexedprimitivesindirectarguments?language=objc
*/
struct DrawIndexedIndirectArguments
{
    //! Specifies the number of indices per instance.
    std::uint32_t   numIndices;

    //! Specifies the number of instances to draw.
    std::uint32_t   numInstances;

    //! Specifies the zero-based offset of the first index from the index buffer.
    std::uint32_t   firstIndex;

    //! Specifies the base vertex offset (positive or negative) which is added to each index from the index buffer.
    std::int32_t    vertexOffset;

    /**
    \brief Specifies the zero-based offset of the first instance.
    \note This parameter modifies the instance ID within the shader pipeline differently for \c SV_InstanceID
    in HLSL and \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan), due to rendering API differences.
    The system value \c SV_InstanceID in HLSL will always start with zero,
    but the system value \c gl_InstanceID in GLSL (or \c gl_InstanceIndex for Vulkan)
    will start with the value of \c firstInstance.
    */
    std::uint32_t   firstInstance;
};

/**
\brief Format structure for the arguments of an indirect draw/indexed-draw command for tessellation on Metal renderer.
\remarks The Metal API treats the arguments for rendering tessellated patches differently,
so this structure is required to fill the buffer that is used for the arguments of an indirect draw command on the Metal backend.
\note Only supported with: Metal.
\see CommandBuffer::DrawIndirect
\see CommandBuffer::DrawIndexedIndirect
\see Metal counterpart \c MTLDrawPatchIndirectArguments: https://developer.apple.com/documentation/metal/mtldrawpatchindirectarguments?language=objc
*/
struct DrawPatchIndirectArguments
{
    //! Specifies the number of patches per instance.
    std::uint32_t numPatches;

    //! Specifies the number of instances to draw.
    std::uint32_t numInstances;

    //! Specifies the patch start index.
    std::uint32_t firstPatch;

    //! Specifies the first instance to draw.
    std::uint32_t firstInstance;
};

/**
\brief Format structure for the arguments of an indirect compute command.
\remarks This structure is byte aligned, i.e. it can be reinterpret casted to a buffer in CPU memory space.
\note This is a plain-old-data (POD) structure, so it has no default constructor to make it easily compatible with the GPU memory space.
\see CommandBuffer::DispatchIndirect
\see OpenGL counterpart \c DispatchIndirectCommand: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDispatchComputeIndirect.xhtml
\see Vulkan counterpart \c VkDispatchIndirectCommand: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkDispatchIndirectCommand.html
\see Direct3D11 counterpart: N/A
\see Direct3D12 counterpart \c D3D12_DISPATCH_ARGUMENTS: https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_dispatch_arguments
\see Metal counterpart \c MTLDispatchThreadgroupsIndirectArguments: https://developer.apple.com/documentation/metal/mtldispatchthreadgroupsindirectarguments?language=objc
*/
struct DispatchIndirectArguments
{
    //! Number of thread groups in X, Y, and Z dimension.
    std::uint32_t numThreadGroups[3];
};

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
