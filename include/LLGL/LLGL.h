/*
 * LLGL.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_INCLUDE_H
#define LLGL_INCLUDE_H


#include "Window.h"
#include "Canvas.h"
#include "Input.h"
#include "Timer.h"
#include "RenderSystem.h"
#include "ColorRGB.h"
#include "ColorRGBA.h"
#include "Desktop.h"
#include "Version.h"


//DOXYGEN MAIN PAGE
/**
 * \mainpage LLGL 0.01 Beta Documentation
 * 
 * LLGL (Low Level Graphics Library)
 * =================================
 * 
 * Overview
 * --------
 * 
 * - **Version**: 0.01 Beta
 * - **License**: [3-Clause BSD License](https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt)
 * 
 * 
 * Progress
 * --------
 * 
 * - **OpenGL Renderer**: ~85% done
 * - **Direct3D 11 Renderer**: ~85% done
 * - **Direct3D 12 Renderer**: ~5% done
 * 
 * 
 * Getting Started
 * ---------------
 * 
 * ```cpp
 * #include <LLGL/LLGL.h>
 * 
 * int main()
 * {
 *     // Create a window to render into
 *     LLGL::WindowDescriptor windowDesc;
 * 
 *     windowDesc.title    = L"LLGL Example";
 *     windowDesc.visible  = true;
 *     windowDesc.centered = true;
 *     windowDesc.width    = 640;
 *     windowDesc.height   = 480;
 * 
 *     auto window = LLGL::Window::Create(windowDesc);
 * 
 *     // Add keyboard/mouse event listener
 *     auto input = std::make_shared<LLGL::Input>();
 *     window->AddEventListener(input);
 * 
 *     //TO BE CONTINUED ...
 * 
 *     // Main loop
 *     while (window->ProcessEvents() && !input->KeyPressed(LLGL::Key::Escape))
 *     {
 *         
 *         // Draw with OpenGL, or Direct3D, or Vulkan, or whatever ...
 *         
 *     }
 *     
 *     return 0;
 * }
 * ```
 * 
 * 
 * Thin Abstraction Layer
 * ----------------------
 * ```cpp
 * // Interface:
 * CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex);
 * 
 * // OpenGL Implementation:
 * void GLCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
 * {
 *     glDrawElements(
 *         renderState_.drawMode,
 *         static_cast<GLsizei>(numVertices),
 *         renderState_.indexBufferDataType,
 *         (reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride))
 *     );
 * }
 * 
 * // Direct3D 11 Implementation
 * void D3D11CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
 * {
 *     context_->DrawIndexed(numVertices, 0, firstIndex);
 * }
 * 
 * // Direct3D 12 Implementation
 * void D3D12CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
 * {
 *     commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
 * }
 * 
 * // Vulkan Implementation
 * void VKCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
 * {
 *     vkCmdDrawIndexed(commandBuffer_, numVertices, 1, firstIndex, 0, 0);
 * }
 * ```
 * 
 */


#endif



// ================================================================================
