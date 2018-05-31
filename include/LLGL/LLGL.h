/*
 * LLGL.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_INCLUDE_H
#define LLGL_INCLUDE_H


#include "Version.h"
#include "Window.h"
#include "Canvas.h"
#include "Display.h"
#include "Input.h"
#include "Timer.h"
#include "ColorRGB.h"
#include "ColorRGBA.h"
#include "RenderSystem.h"


//DOXYGEN MAIN PAGE
/**
 * \mainpage LLGL 0.02 Beta Documentation
 * 
 * LLGL (Low Level Graphics Library)
 * =================================
 * 
 * Overview
 * --------
 * 
 * - **Version**: 0.02 Beta
 * - **License**: [3-Clause BSD License](https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt)
 * 
 * 
 * Progress
 * --------
 * 
 * - **OpenGL Renderer**: ~90% done
 * - **Direct3D 11 Renderer**: ~85% done
 * - **Direct3D 12 Renderer**: ~5% done
 * - **Vulkan Renderer**: ~15% done
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
 * // Unified Interface:
 * CommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex);
 * 
 * // OpenGL Implementation:
 * void GLCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
 * {
 *     const GLsizeiptr indices = firstIndex * renderState_.indexBufferStride;
 *     glDrawElements(
 *         renderState_.drawMode,
 *         static_cast<GLsizei>(numVertices),
 *         renderState_.indexBufferDataType,
 *         reinterpret_cast<const GLvoid*>(indices)
 *     );
 * }
 * 
 * // Direct3D 11 Implementation
 * void D3D11CommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
 * {
 *     context_->DrawIndexed(numVertices, 0, firstIndex);
 * }
 * 
 * // Direct3D 12 Implementation
 * void D3D12CommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
 * {
 *     commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
 * }
 * 
 * // Vulkan Implementation
 * void VKCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
 * {
 *     vkCmdDrawIndexed(commandBuffer_, numVertices, 1, firstIndex, 0, 0);
 * }
 * ```
 * 
 */

/**
\defgroup group_compare_swo Global functions for Strict-Weak-Order (SWO) comparisons.
\defgroup group_operators Global operators for basic data structures.
*/


#endif



// ================================================================================
