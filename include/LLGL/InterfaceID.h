/*
 * InterfaceID.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_INTERFACE_ID_H
#define LLGL_INTERFACE_ID_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Enumeration of all interfaces in LLGL.
\see Interface::IsInstanceOf
\see Interface::AsInstanceOf
*/
struct InterfaceID
{
    enum
    {
        Interface,              //!< \see Interface
        Display,                //!< \see Display
        RenderSystem,           //!< \see RenderSystem
        RenderSystemChild,      //!< \see RenderSystemChild
        Buffer,                 //!< \see Buffer
        BufferArray,            //!< \see BufferArray
        CommandBuffer,          //!< \see CommandBuffer
        CommandQueue,           //!< \see CommandQueue
        Fence,                  //!< \see Fence
        PipelineLayout,         //!< \see PipelineLayout
        PipelineState,          //!< \see PipelineState
        QueryHeap,              //!< \see QueryHeap
        RenderPass,             //!< \see RenderPass
        RenderTarget,           //!< \see RenderTarget
        RenderContext,          //!< \see RenderContext
        Resource,               //!< \see Resource
        ResourceHeap,           //!< \see ResourceHeap
        Sampler,                //!< \see Sampler
        Shader,                 //!< \see Shader
        ShaderProgram,          //!< \see ShaderProgram
        Texture,                //!< \see Texture
        Surface,                //!< \see Surface
        Canvas,                 //!< \see Canvas
        Canvas_EventListener,   //!< \see Canvas::EventListener
        Window,                 //!< \see Window
        Window_EventListener,   //!< \see Window::EventListener
        Input,                  //!< \see Input
        Timer,                  //!< \see Timer

        /**
        \brief Maximum reserved ID for interfaces.
        \remarks Use this value to write custom sub classes of an LLGL interface:
        \code
        class MyWindow : public LLGL::Window {
            LLGL_DECLARE_INTERFACE(LLGL::InterfaceID::Reserved + 1);
            ...
        };
        LLGL_IMPLEMENT_INTERFACE(MyWindow, LLGL::Window);
        \endcode
        */
        Reserved,
    };
};


} // /namespace LLGL


#endif



// ================================================================================
