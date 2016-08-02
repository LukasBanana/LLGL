/*
 * VertexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_VERTEX_FORMAT_H__
#define __LLGL_VERTEX_FORMAT_H__


#include "Export.h"
#include "VertexAttribute.h"
#include <vector>


namespace LLGL
{


class LLGL_EXPORT VertexFormat
{

    public:

        /**
        \brief Adds a new vertex attribute to this vertex format.
        \param[in] name Specifies the attribute name.
        \param[in] dataType Specifies the data type of the attribute components.
        \param[in] components Specifies the number of attribute components. This must be 1, 2, 3, or 4.
        \remarks For HLSL, the name specified the vertex attribute semantics.
        \throws std::invalid_argument If 'components' is neither 1, 2, 3, nor 4.
        */
        void AddAttribute(const std::string& name, const DataType dataType, unsigned int components);

        /**
        \brief Returns the list of all vertex attributes.
        \see AddAttribute
        */
        inline const std::vector<VertexAttribute>& GetAttributes() const
        {
            return attributes_;
        }

        //! Returns the size of this vertex format (in bytes).
        inline unsigned int GetFormatSize() const
        {
            return formatSize_;
        }

    private:
        
        std::vector<VertexAttribute>    attributes_;
        unsigned int                    formatSize_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
