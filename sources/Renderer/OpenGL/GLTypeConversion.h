/*
 * GLTypeConversion.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_TYPE_CONVERSION_H__
#define __LLGL_GL_TYPE_CONVERSION_H__


#include "OpenGL.h"
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{

namespace GLTypeConversion
{


GLenum Map( const BufferUsage       bufferUsage );
GLenum Map( const BufferCPUAccess   cpuAccess   );
GLenum Map( const DataType          dataType    );


} // /namespace GLTypeConversion

} // /namespace LLGL


#endif



// ================================================================================
