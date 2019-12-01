/*
 * GLESProfileTypyes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GLES_PROFILE_TYPES_H
#define LLGL_GLES_PROFILE_TYPES_H


#include "OpenGLES.h"


namespace LLGL
{


#ifndef GL_READ_ONLY
#define GL_READ_ONLY 0x88B8 // for wrappers only
#endif

#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9 // for wrappers only
#endif

#ifndef GL_READ_WRITE
#define GL_READ_WRITE 0x88BA // for wrappers only
#endif

#ifndef GL_LOWER_LEFT
#define GL_LOWER_LEFT 0x8CA1 // for wrappers only
#endif

#ifndef GL_UPPER_LEFT
#define GL_UPPER_LEFT 0x8CA2 // for wrappers only
#endif

#ifndef GL_TEXTURE_1D
#define GL_TEXTURE_1D 0x0DE0 // for wrappers only
#endif

#ifndef GL_TEXTURE_1D_ARRAY
#define GL_TEXTURE_1D_ARRAY 0x8C18 // for wrappers only
#endif

#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE 0x84F5 // for wrappers only
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009 // GLES 3.2
#endif

#ifndef GL_TEXTURE_BUFFER
#define GL_TEXTURE_BUFFER 0x8C2A // GLES 3.2
#endif

#ifndef GL_TEXTURE_2D_MULTISAMPLE
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100 // GLES 3.1
#endif

#ifndef GL_TEXTURE_2D_MULTISAMPLE_ARRAY
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9102 // GLES 3.1
#endif

#ifndef GL_PROXY_TEXTURE_3D
#define GL_PROXY_TEXTURE_3D 0x8070 // for wrappers only
#endif

#ifndef GL_ATOMIC_COUNTER_BUFFER
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0 // GLES 3.2
#endif

#ifndef GL_DISPATCH_INDIRECT_BUFFER
#define GL_DISPATCH_INDIRECT_BUFFER 0x90EE // GLES 3.2
#endif

#ifndef GL_DRAW_INDIRECT_BUFFER
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F // GLES 3.2
#endif

#ifndef GL_QUERY_BUFFER
#define GL_QUERY_BUFFER 0x9192 // for wrappers only
#endif

#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2 // GLES 3.2
#endif

#ifndef GL_STENCIL_INDEX
#define GL_STENCIL_INDEX GL_STENCIL_INDEX8 // indirection
#endif

#ifndef GL_INTERNALFORMAT_SUPPORTED
#define GL_INTERNALFORMAT_SUPPORTED 0x826F // for wrappers only
#endif



// Used for glDepthRangef
typedef GLfloat GLclamp_t;


} // /namespace LLGL


#endif



// ================================================================================
