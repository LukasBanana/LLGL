/*
 * StaticLimits.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STATIC_LIMITS_H
#define LLGL_STATIC_LIMITS_H


//! Maximum number of color attachments allowed for render targets.
#define LLGL_MAX_NUM_COLOR_ATTACHMENTS      (8u)

//! Maximum number of attachments allowed for render targets (color attachments and depth-stencil attachment).
#define LLGL_MAX_NUM_ATTACHMENTS            ((LLGL_MAX_NUM_COLOR_ATTACHMENTS) + 1u)

//! Maximum number of viewports and scissors.
#define LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS (16u)

//! Maximum number of samples for multi-sampled rendering.
#define LLGL_MAX_NUM_SAMPLES                (64u)

//! Maximum number of stream-output buffers.
#define LLGL_MAX_NUM_SO_BUFFERS             (4u)


#endif



// ================================================================================
