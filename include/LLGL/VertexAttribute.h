/*
 * VertexAttribute.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_VERTEX_ATTRIBUTE_H__
#define __LLGL_VERTEX_ATTRIBUTE_H__


#include "Export.h"
#include "Image.h"
#include <string>


namespace LLGL
{


//! Vertex attribute class.
struct LLGL_EXPORT VertexAttribute
{
    VertexAttribute() = default;
    VertexAttribute(const VertexAttribute&) = default;
    VertexAttribute& operator = (const VertexAttribute&) = default;

    /**
    \brief Constructs a vertex attribute with a specified name (used for GLSL).
    \param[in] name Specifies the attribute name (for GLSL).
    \param[in] dataType Specifies the data type of the attribute components.
    \param[in] components Specifies the number of attribute components. This must be 1, 2, 3, or 4.
    \param[in] instanceDivisor Specifies the divisor (or step rate) for instance data.
    If this is 0, this vertex attribute is considered to be per-vertex. By default 0.
    \remarks This is equivalent to:
    \code
    VertexAttribute(name, 0, dataType, components, instanceDivisor);
    \endcode
    */
    VertexAttribute(
        const std::string& name,
        const DataType dataType,
        unsigned int components,
        unsigned int instanceDivisor = 0
    );

    /**
    \brief Constructs a vertex attribute with a specified semantic (used for HLSL).
    \param[in] semanticName Specifies the semantic name (for HLSL).
    \param[in] semanticIndex Specifies the semantic index (for HLSL).
    \param[in] dataType Specifies the data type of the attribute components.
    \param[in] components Specifies the number of attribute components. This must be 1, 2, 3, or 4.
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
        const DataType dataType,
        unsigned int components,
        unsigned int instanceDivisor = 0
    );

    /**
    \brief Returns the size (in bytes) which is required for this vertex attribute.
    \return
    \code
    DataTypeSize(dataType) * components.
    \endcode
    */
    unsigned int GetSize() const;

    //! Vertex attribute name (for GLSL) or semantic name (for HLSL).
    std::string     name;

    //! Data type of the vertex attribute components. By default DataType::Float.
    DataType        dataType        = DataType::Float;

    //! Number of components: 1, 2, 3, or 4. By default 4.
    unsigned int    components      = 4;

    /**
    \brief Specifies the instance data divosor (or instance data step rate).
    \remarks If this is 0, this attribute is considered to be stored per vertex.
    If this is greater than 0, this attribute is considered to be stored per every instanceDivisor's instance.
    */
    unsigned int    instanceDivisor = 0;

    //! Specifies whether non-floating-point data types are to be converted to floating-points. By default false.
    bool            conversion      = false;

    //! Byte offset within each vertex. By default 0.
    unsigned int    offset          = 0;

    //! Semantic index (only relevant for HLSL).
    unsigned int    semanticIndex   = 0;
};


LLGL_EXPORT bool operator == (const VertexAttribute& lhs, const VertexAttribute& rhs);
LLGL_EXPORT bool operator != (const VertexAttribute& lhs, const VertexAttribute& rhs);


} // /namespace LLGL


#endif



// ================================================================================
