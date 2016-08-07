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
#include <LLGL/RenderContextFlags.h>
#include <LLGL/TextureFlags.h>


namespace LLGL
{

namespace GLTypeConversion
{


GLenum Map( const BufferUsage       bufferUsage   );
GLenum Map( const BufferCPUAccess   cpuAccess     );
GLenum Map( const DataType          dataType      );
GLenum Map( const DrawMode          drawMode      );
GLenum Map( const TextureFormat     textureFormat );
GLenum Map( const ColorFormat       colorFormat   );


} // /namespace GLTypeConversion

} // /namespace LLGL


#endif



// ================================================================================
