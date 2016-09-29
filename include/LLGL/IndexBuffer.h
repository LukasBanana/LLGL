/*
 * IndexBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_INDEX_BUFFER_H__
#define __LLGL_INDEX_BUFFER_H__


#include "Export.h"
#include "IndexFormat.h"


namespace LLGL
{


//! Index buffer descriptor structure.
struct IndexBufferDescriptor
{
    IndexBufferDescriptor() = default;

    IndexBufferDescriptor(unsigned int size, BufferUsage usage, const IndexFormat& indexFormat) :
        size        ( size        ),
        usage       ( usage       ),
        indexFormat ( indexFormat )
    {
    }

    //! Buffer size (in bytes).
    unsigned int    size        = 0;

    //! Buffer usage (typically "BufferUsage::Static", since an index buffer is rarely changed).
    BufferUsage     usage       = BufferUsage::Static;

    /**
    \brief Specifies the index format layout, which is basically only the data type of each index.
    \remarks The only valid format types for an index buffer are: DataType::UByte, DataType::UShort, and DataType::UInt.
    \see DataType
    */
    IndexFormat     indexFormat;
};


//! Index buffer interface.
class LLGL_EXPORT IndexBuffer
{

    public:

        virtual ~IndexBuffer()
        {
        }

        inline const IndexFormat& GetIndexFormat() const
        {
            return indexFormat_;
        }

    protected:

        inline void SetIndexFormat(const IndexFormat& indexFormat)
        {
            indexFormat_ = indexFormat;
        }

    private:

        IndexFormat indexFormat_ = IndexFormat(DataType::UInt32);

};


} // /namespace LLGL


#endif



// ================================================================================
