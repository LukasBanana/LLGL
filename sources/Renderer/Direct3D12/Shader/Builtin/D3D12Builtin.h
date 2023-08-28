/*
 * D3D12Builtin.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUILTIN_H
#define LLGL_D3D12_BUILTIN_H

#include "GenerateMips1DCS.inc"
#include "GenerateMips1DCS.OddX.inc"
#include "GenerateMips1DCS.sRGB.inc"
#include "GenerateMips1DCS.sRGB.OddX.inc"

#include "GenerateMips2DCS.inc"
#include "GenerateMips2DCS.OddX.inc"
#include "GenerateMips2DCS.OddY.inc"
#include "GenerateMips2DCS.OddXY.inc"
#include "GenerateMips2DCS.sRGB.inc"
#include "GenerateMips2DCS.sRGB.OddX.inc"
#include "GenerateMips2DCS.sRGB.OddY.inc"
#include "GenerateMips2DCS.sRGB.OddXY.inc"

#include "GenerateMips3DCS.inc"
#include "GenerateMips3DCS.OddX.inc"
#include "GenerateMips3DCS.OddY.inc"
#include "GenerateMips3DCS.OddXY.inc"
#include "GenerateMips3DCS.OddZ.inc"
#include "GenerateMips3DCS.OddXZ.inc"
#include "GenerateMips3DCS.OddYZ.inc"
#include "GenerateMips3DCS.OddXYZ.inc"
#include "GenerateMips3DCS.sRGB.inc"
#include "GenerateMips3DCS.sRGB.OddX.inc"
#include "GenerateMips3DCS.sRGB.OddY.inc"
#include "GenerateMips3DCS.sRGB.OddXY.inc"
#include "GenerateMips3DCS.sRGB.OddZ.inc"
#include "GenerateMips3DCS.sRGB.OddXZ.inc"
#include "GenerateMips3DCS.sRGB.OddYZ.inc"
#include "GenerateMips3DCS.sRGB.OddXYZ.inc"

#define LLGL_IDR_GENERATEMIPS1D_CS              GenerateMips1DCS
#define LLGL_IDR_GENERATEMIPS1D_CS_ODDX         GenerateMips1DCS_OddX
#define LLGL_IDR_GENERATEMIPS1D_CS_SRGB         GenerateMips1DCS_sRGB
#define LLGL_IDR_GENERATEMIPS1D_CS_SRGB_ODDX    GenerateMips1DCS_sRGB_OddX

#define LLGL_IDR_GENERATEMIPS2D_CS              GenerateMips2DCS
#define LLGL_IDR_GENERATEMIPS2D_CS_ODDX         GenerateMips2DCS_OddX
#define LLGL_IDR_GENERATEMIPS2D_CS_ODDY         GenerateMips2DCS_OddY
#define LLGL_IDR_GENERATEMIPS2D_CS_ODDXY        GenerateMips2DCS_OddXY
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB         GenerateMips2DCS_sRGB
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDX    GenerateMips2DCS_sRGB_OddX
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDY    GenerateMips2DCS_sRGB_OddY
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDXY   GenerateMips2DCS_sRGB_OddXY

#define LLGL_IDR_GENERATEMIPS3D_CS              GenerateMips3DCS
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDX         GenerateMips3DCS_OddX
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDY         GenerateMips3DCS_OddY
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDXY        GenerateMips3DCS_OddXY
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDZ         GenerateMips3DCS_OddZ
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDXZ        GenerateMips3DCS_OddXZ
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDYZ        GenerateMips3DCS_OddYZ
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDXYZ       GenerateMips3DCS_OddXYZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB         GenerateMips3DCS_sRGB
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDX    GenerateMips3DCS_sRGB_OddX
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDY    GenerateMips3DCS_sRGB_OddY
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXY   GenerateMips3DCS_sRGB_OddXY
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDZ    GenerateMips3DCS_sRGB_OddZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXZ   GenerateMips3DCS_sRGB_OddXZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDYZ   GenerateMips3DCS_sRGB_OddYZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXYZ  GenerateMips3DCS_sRGB_OddXYZ


#endif