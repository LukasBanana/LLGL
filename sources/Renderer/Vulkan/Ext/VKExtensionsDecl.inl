/*
 * VKExtensionsDecl.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD


#ifndef DECL_VKPROC
#error Missing definition of macro DECL_VKPROC(NAME)
#endif


/* Platform specific VL extensions */

#if defined(LLGL_OS_WIN32)

DECL_VKPROC( vkCreateWin32SurfaceKHR );

#elif defined LLGL_OS_LINUX

DECL_VKPROC( vkCreateXlibSurfaceKHR );

#elif defined LLGL_OS_ANDROID

DECL_VKPROC( vkCreateAndroidSurfaceKHR );

#endif

/* VK_EXT_debug_marker */

DECL_VKPROC( vkDebugMarkerSetObjectTagEXT  );
DECL_VKPROC( vkDebugMarkerSetObjectNameEXT );
DECL_VKPROC( vkCmdDebugMarkerBeginEXT      );
DECL_VKPROC( vkCmdDebugMarkerEndEXT        );
DECL_VKPROC( vkCmdDebugMarkerInsertEXT     );

/* VK_EXT_conditional_rendering */

DECL_VKPROC( vkCmdBeginConditionalRenderingEXT );
DECL_VKPROC( vkCmdEndConditionalRenderingEXT   );

/* VK_EXT_conditional_rendering */

DECL_VKPROC( vkCmdBindTransformFeedbackBuffersEXT );
DECL_VKPROC( vkCmdBeginTransformFeedbackEXT       );
DECL_VKPROC( vkCmdEndTransformFeedbackEXT         );
DECL_VKPROC( vkCmdBeginQueryIndexedEXT            );
DECL_VKPROC( vkCmdEndQueryIndexedEXT              );
DECL_VKPROC( vkCmdDrawIndirectByteCountEXT        );

/* VK_KHR_get_physical_device_properties2 */

DECL_VKPROC( vkGetPhysicalDeviceFeatures2KHR                    );
DECL_VKPROC( vkGetPhysicalDeviceProperties2KHR                  );
DECL_VKPROC( vkGetPhysicalDeviceFormatProperties2KHR            );
DECL_VKPROC( vkGetPhysicalDeviceImageFormatProperties2KHR       );
DECL_VKPROC( vkGetPhysicalDeviceQueueFamilyProperties2KHR       );
DECL_VKPROC( vkGetPhysicalDeviceMemoryProperties2KHR            );
DECL_VKPROC( vkGetPhysicalDeviceSparseImageFormatProperties2KHR );



// ================================================================================
