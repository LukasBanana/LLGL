/*
 * VKExtensionsDecl.inl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

// THIS FILE MUST NOT HAVE A HEADER GUARD

/*
All Vulkan extension functions are declared here.
Depending on the following macros being defined, the respective implementation is enabled:
- LLGL_DEF_VK_EXT_PROCS: defines the global function pointer for Vulkan extensions
- None: declares the global function pointer for Vulkan extensions
*/


#if defined LLGL_DEF_VK_EXT_PROCS

#define DECL_VKPROC(NAME) \
    PFN_##NAME NAME = nullptr

#else

#define DECL_VKPROC(NAME) \
    extern PFN_##NAME NAME

#endif

/* Platform specific VL extensions */

#if defined(LLGL_OS_WIN32)

DECL_VKPROC( vkCreateWin32SurfaceKHR );

#elif defined(LLGL_OS_LINUX)

//???

#endif

#ifdef LLGL_VK_ENABLE_EXT

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

#endif // /LLGL_VK_ENABLE_EXT

#undef DECL_VKPROC



// ================================================================================
