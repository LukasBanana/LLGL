/*
 * LLGL.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
#include "TypeInfo.h"
#include "ColorRGB.h"
#include "ColorRGBA.h"
#include "RenderSystem.h"
#include "Log.h"
#include "IndirectArguments.h"
#include "ImageFlags.h"
#include "VertexFormat.h"


//DOXYGEN MAIN PAGE
/**
 * \mainpage LLGL 0.03 Beta Documentation
 * 
 * LLGL (Low Level Graphics Library)
 * =================================
 * 
 * Overview
 * --------
 * 
 * - **Version**: 0.03 Beta
 * - **License**: [3-Clause BSD License](https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt)
 * 
 * 
 * Progress
 * --------
 * 
 * - **OpenGL Renderer**: ~90% done
 * - **Direct3D 11 Renderer**: ~90% done
 * - **Direct3D 12 Renderer**: ~50% done
 * - **Vulkan Renderer**: ~50% done
 * - **Metal enderer**: ~50% done
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
 * CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex);
 * 
 * // OpenGL Implementation:
 * void GLCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) {
 *     const GLintptr indices = (renderState_.indexBufferOffset + firstIndex * renderState_.indexBufferStride);
 *     glDrawElements(
 *         renderState_.drawMode,
 *         static_cast<GLsizei>(numIndices),
 *         renderState_.indexBufferDataType,
 *         reinterpret_cast<const GLvoid*>(indices)
 *     );
 * }
 * 
 * // Direct3D 11 Implementation
 * void D3D11CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) {
 *     context_->DrawIndexed(numIndices, firstIndex, 0);
 * }
 * 
 * // Direct3D 12 Implementation
 * void D3D12CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) {
 *     commandList_->DrawIndexedInstanced(numIndices, 1, firstIndex, 0, 0);
 * }
 * 
 * // Vulkan Implementation
 * void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) {
 *     vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, 0, 0);
 * }
 * 
 * // Metal implementation
 * void MTCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) {
 *     if (numPatchControlPoints_ > 0) {
 *         [renderEncoder_
 *             drawIndexedPatches:             numPatchControlPoints_
 *             patchStart:                     static_cast<NSUInteger>(firstIndex) / numPatchControlPoints_
 *             patchCount:                     static_cast<NSUInteger>(numIndices) / numPatchControlPoints_
 *             patchIndexBuffer:               nil
 *             patchIndexBufferOffset:         0
 *             controlPointIndexBuffer:        indexBuffer_
 *             controlPointIndexBufferOffset:  indexBufferOffset_ + indexTypeSize_ * (static_cast<NSUInteger>(firstIndex))
 *             instanceCount:                  1
 *             baseInstance:                   0
 *         ];
 *     } else {
 *         [renderEncoder_
 *             drawIndexedPrimitives:  primitiveType_
 *             indexCount:             static_cast<NSUInteger>(numIndices)
 *             indexType:              indexType_
 *             indexBuffer:            indexBuffer_
 *             indexBufferOffset:      indexBufferOffset_ + indexTypeSize_ * static_cast<NSUInteger>(firstIndex)
 *         ];
 *     }
 * }
 * ```
 * 
 */

/**
\defgroup group_compare_swo Global functions for Strict-Weak-Order (SWO) comparisons.
\defgroup group_operators Global operators for basic data structures.
\defgroup group_callbacks Global type aliases to callback interfaces.
*/


#endif



// ================================================================================
