/*
 * Buffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_BUFFER_H__
#define __LLGL_BUFFER_H__


#include "Export.h"
#include "BufferFlags.h"


namespace LLGL
{


//! Hardware buffer interface.
class LLGL_EXPORT Buffer
{

    public:

        virtual ~Buffer()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
