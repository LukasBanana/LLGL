/*
 * OpenXRGraphicsBinding.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_GRAPHICS_BINDING_H
#define LLGL_OPENXR_GRAPHICS_BINDING_H


#include <LLGL/Export.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/Texture.h>
#include <LLGL/XRSystemFlags.h>
#include <LLGL/XRSwapChain.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>

#include "OpenXRPlatform.h"

#include <cstdint>
#include <memory>


namespace LLGL
{


class Report;


namespace OpenXR
{


/**
\brief Per-image entry returned by GraphicsBinding::EnumerateSwapchainImages.
\remarks The texture is owned by the binding (created via the underlying LLGL::RenderSystem) and is released
when the OpenXRSwapChain is destroyed.
*/
struct SwapchainImage
{
    Texture* texture = nullptr;
};

/**
\brief Pure-virtual seam between the backend-agnostic OpenXR core and a graphics-API-specific binding.
\remarks One implementation per supported LLGL renderer (currently only Vulkan).
The graphics binding is responsible for: declaring which OpenXR extensions it needs, creating the LLGL
RenderSystem with the runtime-required device, providing the XrSessionCreateInfo graphics binding chain,
selecting and translating swap-chain formats, and wrapping runtime-owned swap-chain images as LLGL::Texture objects.
*/
class GraphicsBinding
{

    public:

        virtual ~GraphicsBinding() = default;

        //! Returns the LLGL renderer module name this binding supports (e.g. "Vulkan").
        virtual const char* GetRendererModuleName() const = 0;

        //! Returns the OpenXR extensions this graphics binding requires (e.g. {"XR_KHR_vulkan_enable2"}).
        virtual ArrayView<const char*> GetRequiredXrExtensions() const = 0;

        /**
        \brief Creates an LLGL render system whose underlying device satisfies the runtime's binding requirements.
        \return Unique pointer to the render system, or null on failure (reason in \c report).
        */
        virtual RenderSystemPtr CreateRenderSystem(
            XrInstance                          instance,
            XrSystemId                          systemId,
            const XRRenderSystemDescriptor&     renderSystemDesc,
            Report*                             report
        ) = 0;

        /**
        \brief Returns a pointer to the graphics binding chain to assign to XrSessionCreateInfo::next.
        \remarks The returned pointer must remain valid until xrCreateSession returns. The binding owns the underlying storage.
        */
        virtual const void* GetSessionGraphicsBinding(RenderSystem& renderSystem) = 0;

        /**
        \brief Selects a color format supported by the runtime that satisfies the application's preference.
        \param[in] runtimeFormats Array of native format values returned by xrEnumerateSwapchainFormats.
        \param[in] preferred Application's preferred LLGL::Format.
        \param[out] outNativeFormat Receives the native format value (e.g. VkFormat) the runtime should use.
        \return The LLGL::Format selected, or LLGL::Format::Undefined if no suitable format is available.
        */
        virtual Format SelectColorFormat(
            ArrayView<std::int64_t>     runtimeFormats,
            Format                      preferred,
            std::int64_t&               outNativeFormat
        ) const = 0;

        /**
        \brief Selects a depth-stencil format supported by the runtime that satisfies the application's preference.
        \remarks Same contract as SelectColorFormat but filters the runtime-format list down to depth/stencil formats only.
        */
        virtual Format SelectDepthFormat(
            ArrayView<std::int64_t>     runtimeFormats,
            Format                      preferred,
            std::int64_t&               outNativeFormat
        ) const = 0;

        //! Describes whether a swap-chain is a color attachment or a depth-stencil attachment.
        enum class SwapchainKind { Color, DepthStencil };

        /**
        \brief Enumerates the swap-chain images owned by the runtime and wraps them as LLGL::Texture objects.
        \param[in] renderSystem The render system the swap-chain images live on.
        \param[in] swapchain The OpenXR swap-chain handle.
        \param[in] swapChainDesc The application's swap-chain descriptor.
        \param[in] nativeFormat The native format value (from Select*Format).
        \param[in] kind Whether this is a color or depth-stencil swap-chain (controls LLGL bind flags).
        \param[out] outImages Receives the wrapped images.
        \return True on success.
        */
        virtual bool EnumerateSwapchainImages(
            RenderSystem&                       renderSystem,
            XrSwapchain                         swapchain,
            const XRSwapChainDescriptor&        swapChainDesc,
            std::int64_t                        nativeFormat,
            SwapchainKind                       kind,
            SmallVector<SwapchainImage>&        outImages,
            Report*                             report
        ) = 0;

        /**
        \brief Releases a texture previously created via EnumerateSwapchainImages.
        \remarks The runtime image itself is not released here; only the LLGL::Texture wrapper is destroyed.
        */
        virtual void ReleaseSwapchainImage(RenderSystem& renderSystem, Texture& texture) = 0;

};


/**
\brief Creates the Vulkan graphics binding.
\remarks Only declared (and defined) when LLGL is built with both LLGL_BUILD_XR_OPENXR and LLGL_BUILD_RENDERER_VULKAN.
The definition lives in sources/Renderer/Vulkan/OpenXR/VKOpenXRGraphicsBinding.cpp; it is exported from the
LLGL_Vulkan shared library so the (static) LLGL_XR_OpenXR helper library can reach it.
*/
#ifdef LLGL_XR_OPENXR_BIND_VULKAN
LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateVulkanGraphicsBinding();
#endif

//! \brief Creates the Direct3D 11 graphics binding (Windows only).
#ifdef LLGL_XR_OPENXR_BIND_DIRECT3D11
LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateD3D11GraphicsBinding();
#endif

//! \brief Creates the Direct3D 12 graphics binding (Windows only).
#ifdef LLGL_XR_OPENXR_BIND_DIRECT3D12
LLGL_EXPORT std::unique_ptr<GraphicsBinding> CreateD3D12GraphicsBinding();
#endif


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================
