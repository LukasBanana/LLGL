/*
 * InterfaceID.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_INTERFACE_ID_H
#define LLGL_INTERFACE_ID_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Enumeration of all interfaces in LLGL.
\remarks The ID for a base class must always have a smaller value than the ID of its sub classes.
Therefore, all base classes must be declared before their sub classes in this enumeration.
\see Interface::IsInstanceOf
\see Interface::AsInstanceOf
*/
struct InterfaceID
{
    enum
    {
        Interface,              //!< Base class of all public interfaces in LLGL. \see Interface
        Display,                //!< Extends Interface. \see Display
        Surface,                //!< Extends Interface. \see Surface
        Canvas,                 //!< Extends Interface. \see Canvas
        Canvas_EventListener,   //!< Extends Interface. \see Canvas::EventListener
        Window,                 //!< Extends Interface. \see Window
        Window_EventListener,   //!< Extends Interface. \see Window::EventListener
        Input,                  //!< Extends Interface. \see Input
        RenderSystem,           //!< Extends Interface. \see RenderSystem
        RenderSystemChild,      //!< Extends Interface. \see RenderSystemChild
        Buffer,                 //!< Extends RenderSystemChild. \see Buffer
        BufferArray,            //!< Extends RenderSystemChild. \see BufferArray
        CommandBuffer,          //!< Extends RenderSystemChild. \see CommandBuffer
        CommandQueue,           //!< Extends RenderSystemChild. \see CommandQueue
        Fence,                  //!< Extends RenderSystemChild. \see Fence
        PipelineLayout,         //!< Extends RenderSystemChild. \see PipelineLayout
        PipelineState,          //!< Extends RenderSystemChild. \see PipelineState
        QueryHeap,              //!< Extends RenderSystemChild. \see QueryHeap
        RenderPass,             //!< Extends RenderSystemChild. \see RenderPass
        Resource,               //!< Extends RenderSystemChild. \see Resource
        ResourceHeap,           //!< Extends RenderSystemChild. \see ResourceHeap
        Sampler,                //!< Extends RenderSystemChild. \see Sampler
        Shader,                 //!< Extends RenderSystemChild. \see Shader
        Texture,                //!< Extends RenderSystemChild. \see Texture
        RenderTarget,           //!< Extends RenderSystemChild. \see RenderTarget
        SwapChain,              //!< Extends RenderTarget. \see SwapChain

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
