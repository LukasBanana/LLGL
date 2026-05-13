/*
 * NativeHandle.h (OpenXR)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_NATIVE_HANDLE_H
#define LLGL_OPENXR_NATIVE_HANDLE_H


#include <openxr/openxr.h>


namespace LLGL
{

namespace OpenXR
{


/**
\brief Native handle structure for the OpenXR XR system.
\see XRSystem::GetNativeHandle
*/
struct SystemNativeHandle
{
    //! Native handle to the OpenXR instance.
    XrInstance      instance;

    //! Native handle to the OpenXR system id.
    XrSystemId      systemId;
};

/**
\brief Native handle structure for an OpenXR session.
*/
struct SessionNativeHandle
{
    //! Native handle to the OpenXR session.
    XrSession       session;

    //! Native handle to the reference space the session is using for view poses (typically a local or stage space).
    XrSpace         referenceSpace;
};

/**
\brief Native handle structure for an OpenXR swap-chain.
*/
struct SwapChainNativeHandle
{
    //! Native handle to the OpenXR swap-chain.
    XrSwapchain     swapchain;
};


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================
