/*
 * VertexAttribute.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VERTEX_ATTRIBUTE_H
#define LLGL_VERTEX_ATTRIBUTE_H


#include <LLGL/Export.h>
#include <LLGL/Format.h>
#include <LLGL/SystemValue.h>
#include <LLGL/Container/Strings.h>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Vertex input/output attribute structure.
\remarks For attributes within the same vertex buffer, the following members \b must have the same value:
- \c slot
- \c stride
- \c instanceDivisor
\see VertexShaderAttributes::inputAttribs
\see VertexShaderAttributes::outputAttribs
\see BufferDescriptor::vertexAttribs
\see VertexFormat::attributes
\see FragmentAttribute
*/
struct LLGL_EXPORT VertexAttribute
{
    VertexAttribute() = default;
    VertexAttribute(const VertexAttribute&) = default;
    VertexAttribute& operator = (const VertexAttribute&) = default;

    //! Constructor for minimal vertex attribute information and system value semantics, e.g. \c SV_VertexID (HLSL) or \c gl_VertexID (GLSL).
    VertexAttribute(
        const StringView&   name,
        const Format        format,
        std::uint32_t       location        = 0,
        std::uint32_t       instanceDivisor = 0,
        const SystemValue   systemValue     = SystemValue::Undefined
    );

    //! Constructor for basic vertex attribute information.
    VertexAttribute(
        const StringView&   semanticName,
        std::uint32_t       semanticIndex,
        const Format        format,
        std::uint32_t       location        = 0,
        std::uint32_t       instanceDivisor = 0
    );

    //! Constructor for common vertex attribute information.
    VertexAttribute(
        const StringView&   name,
        const Format        format,
        std::uint32_t       location,
        std::uint32_t       offset,
        std::uint32_t       stride,
        std::uint32_t       slot            = 0,
        std::uint32_t       instanceDivisor = 0
    );

    //! Constructor for the most vertex attribute information, including semantic index.
    VertexAttribute(
        const StringView&   semanticName,
        std::uint32_t       semanticIndex,
        const Format        format,
        std::uint32_t       location,
        std::uint32_t       offset,
        std::uint32_t       stride,
        std::uint32_t       slot            = 0,
        std::uint32_t       instanceDivisor = 0
    );

    /**
    \brief Returns the size (in bytes) which is required for this vertex attribute, or zero if the format is not a valid vertex format.
    \see FormatAttributes::bitSize
    \see FormatFlags::SupportsVertex
    \see Format
    */
    std::uint32_t GetSize() const;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    UTF8String      name;

    /**
    \brief Vertex attribute format. By default Format::RGBA32Float.
    \remarks Not all hardware formats are allowed for vertex attributes.
    In particular, depth-stencil formats and compressed formats are not allowed.
    To specify a vertex attribute of a matrix type, multiple attributes with ascending semantic indices must be used.
    Here is an example of a 4x4 matrix:
    \code
    myVertexFormat.AppendAttribute({ "myMatrix4x4", 0, LLGL::Format::RGBA32Float });
    myVertexFormat.AppendAttribute({ "myMatrix4x4", 1, LLGL::Format::RGBA32Float });
    myVertexFormat.AppendAttribute({ "myMatrix4x4", 2, LLGL::Format::RGBA32Float });
    myVertexFormat.AppendAttribute({ "myMatrix4x4", 3, LLGL::Format::RGBA32Float });
    \endcode
    Here is an example of a 2x2 matrix:
    \code
    myVertexFormat.AppendAttribute({ "myMatrix2x2", 0, LLGL::Format::RG32Float });
    myVertexFormat.AppendAttribute({ "myMatrix2x2", 1, LLGL::Format::RG32Float });
    \endcode
    */
    Format          format          = Format::RGBA32Float;

    /**
    \brief Vertex attribute location (only for OpenGL, Vulkan, Metal) or stream-output number (only for Direct3D).
    \remarks This is only required for OpenGL, Vulkan, and Metal. For Direct3D, this is ignored and instead \c semanticIndex is used.
    \remarks The following example shows GLSL attribute locations from 0 to 4 inclusive:
    \code
    layout(location = 0) in vec4 vertexPosition;   // location 0
    layout(location = 1) in mat4 projectionMatrix; // location 1...4
    \endcode
    \remarks The following example shows Metal attribute locations from 0 to 4 inclusive:
    \code
    struct MyVertexInput
    {
        float4 vertexPosition    [[attribute(0)]];
        float4 projectionMatrix0 [[attribute(1)]];
        float4 projectionMatrix1 [[attribute(2)]];
        float4 projectionMatrix2 [[attribute(3)]];
        float4 projectionMatrix3 [[attribute(4)]];
    };
    \endcode
    */
    std::uint32_t   location        = 0;

    /**
    \brief Semantic index for HLSL.
    \remarks This is only required for Direct3D when a semantic name is used multiple times.
    This happens when a matrix type is distributed over multiple vector attributes.
    \remarks The following example uses semantic names \c POS0, \c MATRIX0, \c MATRIX1, \c MATRIX2, \c MATRIX3:
    \code
    struct MyVertexInput
    {
        float4   vertexPosition   : POS;
        float4x4 projectionMatrix : MATRIX;
    };
    \endcode
    */
    std::uint32_t   semanticIndex   = 0;

    /**
    \brief Specifies the system value type for this vertex attribute or SystemValue::Undefined if this attribute is not a system value. By default SystemValue::Undefined.
    \remarks System value semantics are only used for shader code reflection. Examples of system value semantics are:
    - Vertex ID: \c SV_VertexID (HLSL), \c gl_VertexID (GLSL), \c gl_VertexIndex (SPIR-V), <code>[[vertex_id]]</code> (Metal).
    - Instance ID: \c SV_InstanceID (HLSL), \c gl_InstanceID (GLSL), \c gl_InstanceIndex (SPIR-V), <code>[[instance_id]]</code> (Metal).
    \see Shader::Reflect
    */
    SystemValue     systemValue     = SystemValue::Undefined;

    /**
    \brief Vertex buffer binding slot. By default 0.
    \remarks This is used when multiple vertex buffers are used simultaneously.
    This binding slot refers either to the input buffer indices (determined by \c SetVertexBufferArray),
    or stream-output buffer indices (determined by \c BeginStreamOutput).
    \note Only supported with: Direct3D 11, Direct3D 12, Vulkan, Metal.
    \note For OpenGL, the vertex binding slots are automatically generated in ascending order and beginning with zero.
    \see CommandBuffer::SetVertexBufferArray
    \see CommandBuffer::BeginStreamOutput
    */
    std::uint32_t   slot            = 0;

    /**
    \brief Byte offset within each vertex and each buffer for input attributes, or component offset for output attributes. By default 0.
    \remarks For vertex input attributes, this offset specifies the byte aligned offset within each vertex buffer.
    \remarks For stream-output attributes, this offset specifies the first component that is to be written and must be either 0, 1, 2, or 3.
    */
    std::uint32_t   offset          = 0;

    //! Specifies the vertex data stride which describes the byte offset between consecutive vertices.
    std::uint32_t   stride          = 0;

    /**
    \brief Instance data divisor (or instance data step rate).
    \remarks If this is 0, this attribute is considered to be stored per vertex.
    If this is greater than 0, this attribute is considered to be stored per every instanceDivisor's instance.
    \note For Vulkan, this must only be 0 or 1.
    */
    std::uint32_t   instanceDivisor = 0;
};


/* ----- Operator ----- */

//! Compares the two VertexAttribute types for equality (including their names and all other members).
LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs);

//! Compares the two VertexAttribute types for inequality (including their names and all other members).
LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs);


} // /namespace LLGL


#endif



// ================================================================================
