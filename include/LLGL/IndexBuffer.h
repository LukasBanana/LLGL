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
