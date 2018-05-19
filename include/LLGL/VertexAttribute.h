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
    \param[in] vectorType Specifies the vector type of the attribute.
    \param[in] instanceDivisor Specifies the divisor (or step rate) for instance data.
    If this is 0, this vertex attribute is considered to be per-vertex. By default 0.
    \remarks This is equivalent to:
    \code
    VertexAttribute(name, 0, dataType, components, instanceDivisor);
    \endcode
    */
    VertexAttribute(
        const std::string& name,
        const VectorType vectorType,
        std::uint32_t instanceDivisor = 0
    );

    /**
    \brief Constructs a vertex attribute with a specified semantic (used for HLSL).
    \param[in] semanticName Specifies the semantic name (for HLSL).
    \param[in] semanticIndex Specifies the semantic index (for HLSL).
    \param[in] vectorType Specifies the vector type of the attribute.
    \param[in] instanceDivisor Specifies the divisor (or step rate) for instance data.
    If this is 0, this vertex attribute is considered to be per-vertex. By default 0.
    \remarks This is equivalent to:
    \code
    VertexAttribute(name, 0, dataType, components, instanceDivisor);
    \endcode
    */
    VertexAttribute(
        const std::string& semanticName,
        std::uint32_t semanticIndex,
        const VectorType vectorType,
        std::uint32_t instanceDivisor = 0
    );

    /**
    \brief Returns the size (in bytes) which is required for this vertex attribute.
    \return
    \code
    VectorTypeSize(vectorType).
    \endcode
    */
    std::uint32_t GetSize() const;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    std::string     name;

    /**
    \brief Vector type of the vertex attribute. By default VectorType::Float4.
    \note The double types are only supported with OpenGL, i.e. the vector types:
    VectorType::Double, VectorType::Double2, VectorType::Double3, and VectorType::Double4.
    \remarks To specifies an attribute of a matrix type, use multiple vector types with ascending values for the semanticIndex field.
    Here is an example for a 4x4 matrix type:
    \code
    myVertexFormat.AppendAttribute({ "myMatrix", 0, LLGL::VectorType::Float4 });
    myVertexFormat.AppendAttribute({ "myMatrix", 1, LLGL::VectorType::Float4 });
    myVertexFormat.AppendAttribute({ "myMatrix", 2, LLGL::VectorType::Float4 });
    myVertexFormat.AppendAttribute({ "myMatrix", 3, LLGL::VectorType::Float4 });
    \endcode
    \see semanticIndex
    */
    VectorType      vectorType      = VectorType::Float4;

    /**
    \brief Instance data divisor (or instance data step rate).
    \remarks If this is 0, this attribute is considered to be stored per vertex.
    If this is greater than 0, this attribute is considered to be stored per every instanceDivisor's instance.
    */
    std::uint32_t   instanceDivisor = 0;

    //! Specifies whether non-floating-point data types are to be converted to floating-points. By default false.
    bool            conversion      = false;

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
