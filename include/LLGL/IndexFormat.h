/*
 * IndexFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_INDEX_FORMAT_H
#define LLGL_INDEX_FORMAT_H


#include "Export.h"
#include "Image.h"


namespace LLGL
{


/**
\brief Index buffer format class.
\see BufferDescriptor::IndexBuffer
\todo Remove this class. Should be handled by individual DataType members and then class 'DataTypeSize' when necessary.
*/
class LLGL_EXPORT IndexFormat
{

    public:

        IndexFormat() = default;
        IndexFormat(const IndexFormat&) = default;
        IndexFormat& operator = (const IndexFormat&) = default;

        /**
        \brief Constructor to initialize the index format with the specified data type.
        \remarks This will automatically determine the format size.
        \see GetFormatSize
        */
        IndexFormat(const DataType dataType);

        //! Returns the data type of this index format.
        inline DataType GetDataType() const
        {
            return dataType_;
        }

        //! Returns the size of this vertex format (in bytes).
        inline std::uint32_t GetFormatSize() const
        {
            return formatSize_;
        }

    private:

        DataType        dataType_   = DataType::UInt32;
        std::uint32_t   formatSize_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
