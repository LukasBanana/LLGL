/*
 * VertexAttribute.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VERTEX_ATTRIBUTE_H
#define LLGL_VERTEX_ATTRIBUTE_H


#include "Export.h"
#include "Format.h"
#include <string>


namespace LLGL
{


//! Vertex attribute structure.
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
        unsigned int instanceDivisor = 0
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
        unsigned int semanticIndex,
        const VectorType vectorType,
        unsigned int instanceDivisor = 0
    );

    /**
    \brief Returns the size (in bytes) which is required for this vertex attribute.
    \return
    \code
    VectorTypeSize(vectorType).
    \endcode
    */
    unsigned int GetSize() const;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    std::string     name;

    /**
    \brief Vector type of the vertex attribute. By default VectorType::Float4.
    \remarks The double types are only supported with OpenGL, i.e. the vector types:
    VectorType::Double, VectorType::Double2, VectorType::Double3, and VectorType::Double4.
    */
    VectorType      vectorType      = VectorType::Float4;

    /**
    \brief Instance data divisor (or instance data step rate).
    \remarks If this is 0, this attribute is considered to be stored per vertex.
    If this is greater than 0, this attribute is considered to be stored per every instanceDivisor's instance.
    */
    unsigned int    instanceDivisor = 0;

    //! Specifies whether non-floating-point data types are to be converted to floating-points. By default false.
    bool            conversion      = false;

    //! Byte offset within each vertex. By default 0.
    unsigned int    offset          = 0;

    /**
    \brief Semantic index.
    \note Only supported with: Direct3D 11, Direct3D 12.
    */
    unsigned int    semanticIndex   = 0;

    /**
    \brief Vertex buffer input slot.
    \remarks This is used when multiple vertex buffers are used simultaneously.
    */
    unsigned int    inputSlot       = 0;
};


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs);
LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs);


} // /namespace LLGL


#endif



// ================================================================================
