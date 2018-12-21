/*
 * StaticLimits.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STATIC_LIMITS_H
#define LLGL_STATIC_LIMITS_H


// Maximum number of color attachments allowed for render targets.
#define LLGL_MAX_NUM_COLOR_ATTACHMENTS      (8u)

// Maximum number of attachments allowed for render targets (color attachments and depth-stencil attachment).
#define LLGL_MAX_NUM_ATTACHMENTS            ((LLGL_MAX_NUM_COLOR_ATTACHMENTS) + 1u)

// Maximum number of viewports and scissors.
#define LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS (16u)


#endif



// ================================================================================
