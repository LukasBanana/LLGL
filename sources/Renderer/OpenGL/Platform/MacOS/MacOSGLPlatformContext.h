/*
 * MacOSGLPlatformContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_MACOS_GL_PLATFORM_CONTEXT_H__
#define __LLGL_MACOS_GL_PLATFORM_CONTEXT_H__


#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>


namespace LLGL
{


struct GLPlatformContext
{
    //NSOpenGLContext*    glc;
    CGLContextObj ctx;
};


} // /namespace LLGL


#endif



// ================================================================================
