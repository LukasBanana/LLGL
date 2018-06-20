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


/**
\brief Vertex attribute structure.
\see VertexFormat
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
    To specify a vertex attribute of a matrix type, multiple attributes with ascending semantic indices must be used:
    \code
    myVertexFormat.AppendAttribute({ "myMatrix", 0, LLGL::Format::RGBA32Float });
    myVertexFormat.AppendAttribute({ "myMatrix", 1, LLGL::Format::RGBA32Float });
    myVertexFormat.AppendAttribute({ "myMatrix", 2, LLGL::Format::RGBA32Float });
    myVertexFormat.AppendAttribute({ "myMatrix", 3, LLGL::Format::RGBA32Float });
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
};


//! Compares the two VertexAttribute types for equality (including their names and all other members).
LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs);

//! Compares the two VertexAttribute types for inequality (including their names and all other members).
LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs);


} // /namespace LLGL


#endif



// ================================================================================
