/*
 * BrickPlay.Const.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_CONST_H
#define LLGL_EXAMPLE_BRICKPLAY_CONST_H


#include "BrickPlay.Brick.h"
#include <LLGL/Utils/ColorRGBA.h>
#include <cstddef>


struct Cfg
{
    static constexpr bool           useCameraAcceleration           = true;
    static constexpr float          cameraTurnSpeed                 = 0.25f;
    static constexpr float          cameraMoveSpeed                 = 2.5f;
    static constexpr float          cameraMass                      = 0.1f;     // Camera mass for physics based movement.
    static constexpr float          cameraDampingCeofficient        = 0.99f;
    static constexpr float          cameraCollisionRadius           = 1.5f;

    static constexpr float          previewBlockOffsetY             = 0.075f;   // Y-axis world position offset for the preview block before putting it into place

    static constexpr float          silhouetteColor[3]              = { 1.0f, 1.0f, 0.0f }; // Color of the silhouette outline for selectin bricks
    static constexpr float          silhouetteWidth                 = 5.0f;

    static constexpr float          uiMenuTriggerDistCoeff          = 1.0f; // Percentage to menu bar width of how many pixels the user has to drag the menu bar until it starts scrolling or pulling out bricks

    static constexpr float          uiScrollSpeed                   = 50.0f;
    static constexpr int            uiScrollIndicatorSize           = 15;

    static const LLGL::ColorRGBAub  uiBrickThumbnailBgColor;
    static const LLGL::ColorRGBAub  uiSideMenuColor;
    static constexpr GridRotation   uiThumbnailDefaultRotation      = GridRotation_270;
    static constexpr float          uiThumbnailMinVolumeSize        = 3.0f;     // Minimum size for a bounding volume of brick thumbnails.

    static constexpr int            maxBrickExtent                  = 16;       // Limit how large bricks can be in X/Y/Z dimension

    static constexpr int            planeSubdivisions               = 1;

    static constexpr int            heightFieldChunkSize            = 2;        // Build bricks from height field in 4x4 grid chunks so that the largest bricks can be utilized

    static constexpr std::size_t    maxHistoryLength                = 50;       // Maxmimum number of steps to undo

    // Whether to use extended brick models for their optimized versions (*_WallsOnlyExt.obj).
    // If false, covered bricks can show tiny holes due to beveled edges of bricks sitting above and below them.
    // This can be turned off for mobile platforms where this flaw is virtually invisible
    // and the incurred performance cost of more almost invisible geometry can be avoided.
    static constexpr bool           useWaterTightBrickOptimization  = true;
}; // /namespace Cfg

const LLGL::ColorRGBAub  Cfg::uiBrickThumbnailBgColor = { 180, 180, 180, 255 };
const LLGL::ColorRGBAub  Cfg::uiSideMenuColor         = { 0, 0, 0, 180 };


#endif // /LLGL_EXAMPLE_BRICKPLAY_UTILS_H


