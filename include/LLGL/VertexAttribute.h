/*
 * VertexAttribute.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VERTEX_ATTRIBUTE_H
#define LLGL_VERTEX_ATTRIBUTE_H


#include "Export.h"
#include "Format.h"

#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Enumeration of system values available in a shader program.
\remarks This is only used for shader code reflection.
\see VertexAttribute::systemValue
*/
enum class SystemValue
{
    //! Undefined system value.
    Undefined,

    /**
    \brief Forward-compatible mechanism for vertex clipping.
    \remarks HLSL: \c SV_ClipDistance, GLSL and SPIR-V: \c gl_ClipDistance, Metal: <code>[[clip_distance]]</code>.
    */
    ClipDistance,

    /**
    \brief Mechanism for controlling user culling.
    \remarks HLSL: \c SV_CullDistance, GLSL and SPIR-V: \c gl_CullDistance.
    */
    CullDistance,

    /**
    \brief Indicates whether a primitive is front or back facing.
    \remarks HLSL: \c SV_IsFrontFace, GLSL and SPIR-V: \c gl_FrontFacing, Metal: <code>[[front_facing]]</code>.
    */
    FrontFacing,

    /**
    \brief Index of the input instance.
    \remarks HLSL: \c SV_InstanceID, GLSL: \c gl_InstanceID, SPIR-V: \c gl_InstanceIndex, Metal: <code>[[instance_id]]</code>.
    \note This value behalves differently between Direct3D and OpenGL.
    \see CommandBuffer::DrawInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
    */
    InstanceID,

    /**
    \brief Vertex or fragment position.
    \remarks HLSL: \c SV_Position,
    GLSL and SPIR-V in a vertex shader: \c gl_Position,
    GLSL and SPIR-V in a fragment shader: \c gl_FragCoord,
    Metal: <code>[[position]]</code>.
    */
    Position,

    /**
    \brief Index of the geometry primitive.
    \remarks HLSL: \c SV_PrimitiveID, GLSL and SPIR-V: \c gl_PrimitiveID.
    */
    PrimitiveID,

    /**
    \brief Index of the render target layer.
    \remarks HLSL: \c SV_RenderTargetArrayIndex, GLSL and SPIR-V: \c gl_Layer, Metal: <code>[[render_target_array_index]]</code>.
    */
    RenderTargetIndex,

    /**
    \brief Index of the input sample.
    \remarks HLSL: \c SV_SampleIndex, GLSL and SPIR-V: \c gl_SampleID, Metal: <code>[[sample_id]]</code>.
    */
    SampleID,

    /**
    \brief Render target output value.
    \remarks HLSL: \c SV_Target, GLSL and SPIR-V: N/A.
    */
    Target,

    /**
    \brief Index of the input vertex.
    \remarks HLSL: \c SV_VertexID, GLSL: \c gl_VertexID, SPIR-V: \c gl_VertexIndex, Metal: <code>[[vertex_id]]</code>.
    \note This value behalves differently between Direct3D and OpenGL.
    \see CommandBuffer::Draw
    */
    VertexID,

    /**
    \brief Index of the viewport array.
    \remarks HLSL: \c SV_ViewportArrayIndex, GLSL and SPIR-V: \c gl_ViewportIndex, Metal: <code>[[viewport_array_index]]</code>.
    */
    ViewportIndex,
};


/* ----- Structures ----- */

/**
\brief Vertex attribute structure.
\see VertexFormat::attributes
*/
struct LLGL_EXPORT VertexAttribute
{
    VertexAttribute() = default;
    VertexAttribute(const VertexAttribute&) = default;
    VertexAttribute& operator = (const VertexAttribute&) = default;

    /**
    \brief Constructs a vertex attribute with a specified name (used for GLSL).
    \param[in] name Specifies the attribute name (for GLSL).
    \param[in] format Specifies the attribute format (interpreted as vector format rather than color format).
    For a 3D-vector type, for example, Format::RGB32Float can be used.
    \param[in] instanceDivisor Specifies the divisor (or step rate) for instance data.
    If this is 0, this vertex attribute is considered to be per-vertex. By default 0.
    \remarks This is equivalent to:
    \code
    VertexAttribute(name, 0, dataType, components, instanceDivisor);
    \endcode
    \see Format
    */
    VertexAttribute(
        const std::string&  name,
        const Format        format,
        std::uint32_t       instanceDivisor = 0
    );

    /**
    \brief Constructs a vertex attribute with a specified semantic (used for HLSL).
    \param[in] semanticName Specifies the semantic name (for HLSL).
    \param[in] semanticIndex Specifies the semantic index (for HLSL).
    \param[in] format Specifies the attribute format (interpreted as vector format rather than color format).
    For a 3D-vector type, for example, Format::RGB32Float can be used.
    \param[in] instanceDivisor Specifies the divisor (or step rate) for instance data.
    If this is 0, this vertex attribute is considered to be per-vertex. By default 0.
    \remarks This is equivalent to:
    \code
    VertexAttribute(name, 0, dataType, components, instanceDivisor);
    \endcode
    \see Format
    */
    VertexAttribute(
        const std::string&  semanticName,
        std::uint32_t       semanticIndex,
        const Format        format,
        std::uint32_t       instanceDivisor = 0
    );

    /**
    \brief Returns the size (in bytes) which is required for this vertex attribute.
    \return The format bit size converted to byte size: <code>FormatBitSize(format) / 8</code>.
    */
    std::uint32_t GetSize() const;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    std::string     name;

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
    \brief Instance data divisor (or instance data step rate).
    \remarks If this is 0, this attribute is considered to be stored per vertex.
    If this is greater than 0, this attribute is considered to be stored per every instanceDivisor's instance.
    */
    std::uint32_t   instanceDivisor = 0;

    //! Byte offset within each vertex and each buffer. By default 0.
    std::uint32_t   offset          = 0;

    /**
    \brief Semantic index (for HLSL) or vector index (for GLSL).
    \remarks This is used when a matrix is distributed over multiple vector attributes.
    */
    std::uint32_t   semanticIndex   = 0;

    /**
    \brief Specifies the system value type for this vertex attribute or SystemValue::Undefined if this attribute is not a system value. By default SystemValue::Undefined.
    \remarks System value semantics must not be specified to create a shader program.
    Instead, they are used only for shader code reflection. Examples of system value semantics are:
    - Vertex ID: \c SV_VertexID (HLSL), \c gl_VertexID (GLSL), \c gl_VertexIndex (SPIR-V).
    - Instance ID: \c SV_InstanceID (HLSL), \c gl_InstanceID (GLSL), \c gl_InstanceIndex (SPIR-V).
    \see ShaderProgram::QueryReflectionDesc
    */
    SystemValue     systemValue     = SystemValue::Undefined;
};


//! Compares the two VertexAttribute types for equality (including their names and all other members).
LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs);

//! Compares the two VertexAttribute types for inequality (including their names and all other members).
LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs);


} // /namespace LLGL


#endif



// ================================================================================
