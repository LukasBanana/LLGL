/*
 * CommandBuffer.Drawing.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Drawing ----- */

virtual void Draw(
    std::uint32_t   numVertices,
    std::uint32_t   firstVertex
) override final;

virtual void DrawIndexed(
    std::uint32_t   numIndices,
    std::uint32_t   firstIndex
) override final;

virtual void DrawIndexed(
    std::uint32_t   numIndices,
    std::uint32_t   firstIndex,
    std::int32_t    vertexOffset
) override final;

virtual void DrawInstanced(
    std::uint32_t   numVertices,
    std::uint32_t   firstVertex,
    std::uint32_t   numInstances
) override final;

virtual void DrawInstanced(
    std::uint32_t   numVertices,
    std::uint32_t   firstVertex,
    std::uint32_t   numInstances,
    std::uint32_t   firstInstance
) override final;

virtual void DrawIndexedInstanced(
    std::uint32_t   numIndices,
    std::uint32_t   numInstances,
    std::uint32_t   firstIndex
) override final;

virtual void DrawIndexedInstanced(
    std::uint32_t   numIndices,
    std::uint32_t   numInstances,
    std::uint32_t   firstIndex,
    std::int32_t    vertexOffset
) override final;

virtual void DrawIndexedInstanced(
    std::uint32_t   numIndices,
    std::uint32_t   numInstances,
    std::uint32_t   firstIndex,
    std::int32_t    vertexOffset,
    std::uint32_t   firstInstance
) override final;

virtual void DrawIndirect(
    LLGL::Buffer&   buffer,
    std::uint64_t   offset
) override final;

virtual void DrawIndirect(
    LLGL::Buffer&   buffer,
    std::uint64_t   offset,
    std::uint32_t   numCommands,
    std::uint32_t   stride
) override final;

virtual void DrawIndexedIndirect(
    LLGL::Buffer&   buffer,
    std::uint64_t   offset
) override final;

virtual void DrawIndexedIndirect(
    LLGL::Buffer&   buffer,
    std::uint64_t   offset,
    std::uint32_t   numCommands,
    std::uint32_t   stride
) override final;



// ================================================================================
