/*
 * GLDriverBugMitigation.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_DRIVER_BUG_MITIGATION_H
#define LLGL_GL_DRIVER_BUG_MITIGATION_H


/*
This file contains macro switches that enable/disable mitigations and workarounds in LLGL for driver bugs.
Whether this is caused by a driver bug, poor GL practices, or an issue on LLGL's side is open to debate,
but these are the conditions under which the authors consider the issue to be a driver bug which is beyond LLGL's control:

1. Thorough logging using glGet* functions to query the GL state indicates that all states are as expected,
   unless the states clearly violate the GL spec, but the result is still wrong.

2. RenderDoc shows the issue only in its thumbnail of a frame capture, which comes from the GPU driver but the issue does not reproduce in the RenderDoc replay.
   Since RenderDoc uses its own state tracking, this is a clue that the driver might not have followed the GL specification
   as RenderDoc does (the other way around is a possibility, too, of course).

3. A small standalone test program can reproduce the issue on one GPU but not on another vendor's GPU.
*/


#define LLGL_OPTION_DISABLED                        0
#define LLGL_OPTION_ENABLED                         1
#define LLGL_OPTION_ENABLED_FOR_SPECIFIC_GPU_ONLY   2


/*
NVIDIA GL driver bug (observed with 617.14, RTX 4070 SUPER; not reproducible on Intel Arc):
After glDeleteProgram(), the next glCreateProgram() may return the same program name. If the new
program uses a vertex attribute that was inactive (optimized out) in the old program with that name,
and it draws with a VAO that was previously used with the old program, the driver does not fetch
that attribute from the VAO. The shader reads the generic default (0,0,0,1) instead.
glFinish() before deleting does not help. Not deleting the program (no name reuse) or drawing with
a newly generated VAO avoids it. LLGL therefore keeps released GL programs alive instead of
deleting them immediately.
*/
#define LLGL_GL_DRIVER_BUG_NVIDIA_glDeleteProgram ( LLGL_OPTION_ENABLED_FOR_SPECIFIC_GPU_ONLY )

/*
This enables buffer-write hazard tracking for WebGL.
The Metal backend of the WebGL implementation "ANGLE" does not correctly track mid-frame buffer updates in `glBufferSubData()`.
This option enables LLGL's own hazard tracking using staging buffer pools for dynamic buffer updates in the GL backend.
*/
#if LLGL_WEBGL
#   define LLGL_GL_DRIVER_BUG_ANGLE_glBufferSubData ( LLGL_OPTION_ENABLED )
#else
#   define LLGL_GL_DRIVER_BUG_ANGLE_glBufferSubData ( LLGL_OPTION_DISABLED )
#endif


#endif



// ================================================================================
