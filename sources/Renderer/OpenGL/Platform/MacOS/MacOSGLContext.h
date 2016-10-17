/*
 * MacOSGLContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_MACOS_GL_CONTEXT_H__
#define __LLGL_MACOS_GL_CONTEXT_H__


#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <AppKit/NSOpenGL.h>

#include <LLGL/Platform/NativeHandle.h>
#include "../GLContext.h"


namespace LLGL
{

    
class MacOSGLContext : public GLContext
{
    
    public:
        
        MacOSGLContext(RenderContextDescriptor& desc, Window& window, MacOSGLContext* sharedContext);
        ~MacOSGLContext();
        
        bool SetSwapInterval(int interval) override;
        bool SwapBuffers() override;
        
    private:
        
        bool Activate(bool activate) override;
        
        void CreateContext(const NativeHandle& nativeHandle, MacOSGLContext* sharedContext);
        void DeleteContext();
    
        NSOpenGLPixelFormat*    pixelFormat_ = nullptr;
        CGLContextObj           ctx_;
    
};
    

} // /namespace LLGL


#endif



// ================================================================================
