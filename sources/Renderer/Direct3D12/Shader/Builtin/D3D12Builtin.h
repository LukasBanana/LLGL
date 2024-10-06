/*
 * D3D12Builtin.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_BUILTIN_H
#define LLGL_D3D12_BUILTIN_H

#include "GenerateMips1DCS.cso.inl"
#include "GenerateMips1DCS.OddX.cso.inl"
#include "GenerateMips1DCS.sRGB.cso.inl"
#include "GenerateMips1DCS.sRGB.OddX.cso.inl"

#include "GenerateMips2DCS.cso.inl"
#include "GenerateMips2DCS.OddX.cso.inl"
#include "GenerateMips2DCS.OddY.cso.inl"
#include "GenerateMips2DCS.OddXY.cso.inl"
#include "GenerateMips2DCS.sRGB.cso.inl"
#include "GenerateMips2DCS.sRGB.OddX.cso.inl"
#include "GenerateMips2DCS.sRGB.OddY.cso.inl"
#include "GenerateMips2DCS.sRGB.OddXY.cso.inl"

#include "GenerateMips3DCS.cso.inl"
#include "GenerateMips3DCS.OddX.cso.inl"
#include "GenerateMips3DCS.OddY.cso.inl"
#include "GenerateMips3DCS.OddXY.cso.inl"
#include "GenerateMips3DCS.OddZ.cso.inl"
#include "GenerateMips3DCS.OddXZ.cso.inl"
#include "GenerateMips3DCS.OddYZ.cso.inl"
#include "GenerateMips3DCS.OddXYZ.cso.inl"
#include "GenerateMips3DCS.sRGB.cso.inl"
#include "GenerateMips3DCS.sRGB.OddX.cso.inl"
#include "GenerateMips3DCS.sRGB.OddY.cso.inl"
#include "GenerateMips3DCS.sRGB.OddXY.cso.inl"
#include "GenerateMips3DCS.sRGB.OddZ.cso.inl"
#include "GenerateMips3DCS.sRGB.OddXZ.cso.inl"
#include "GenerateMips3DCS.sRGB.OddYZ.cso.inl"
#include "GenerateMips3DCS.sRGB.OddXYZ.cso.inl"

#include "StreamOutputDrawArgs.dxil.inl"

#define LLGL_IDR_GENERATEMIPS1D_CS              g_GenerateMips1DCS
#define LLGL_IDR_GENERATEMIPS1D_CS_ODDX         g_GenerateMips1DCS_OddX
#define LLGL_IDR_GENERATEMIPS1D_CS_SRGB         g_GenerateMips1DCS_sRGB
#define LLGL_IDR_GENERATEMIPS1D_CS_SRGB_ODDX    g_GenerateMips1DCS_sRGB_OddX

#define LLGL_IDR_GENERATEMIPS2D_CS              g_GenerateMips2DCS
#define LLGL_IDR_GENERATEMIPS2D_CS_ODDX         g_GenerateMips2DCS_OddX
#define LLGL_IDR_GENERATEMIPS2D_CS_ODDY         g_GenerateMips2DCS_OddY
#define LLGL_IDR_GENERATEMIPS2D_CS_ODDXY        g_GenerateMips2DCS_OddXY
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB         g_GenerateMips2DCS_sRGB
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDX    g_GenerateMips2DCS_sRGB_OddX
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDY    g_GenerateMips2DCS_sRGB_OddY
#define LLGL_IDR_GENERATEMIPS2D_CS_SRGB_ODDXY   g_GenerateMips2DCS_sRGB_OddXY

#define LLGL_IDR_GENERATEMIPS3D_CS              g_GenerateMips3DCS
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDX         g_GenerateMips3DCS_OddX
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDY         g_GenerateMips3DCS_OddY
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDXY        g_GenerateMips3DCS_OddXY
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDZ         g_GenerateMips3DCS_OddZ
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDXZ        g_GenerateMips3DCS_OddXZ
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDYZ        g_GenerateMips3DCS_OddYZ
#define LLGL_IDR_GENERATEMIPS3D_CS_ODDXYZ       g_GenerateMips3DCS_OddXYZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB         g_GenerateMips3DCS_sRGB
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDX    g_GenerateMips3DCS_sRGB_OddX
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDY    g_GenerateMips3DCS_sRGB_OddY
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXY   g_GenerateMips3DCS_sRGB_OddXY
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDZ    g_GenerateMips3DCS_sRGB_OddZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXZ   g_GenerateMips3DCS_sRGB_OddXZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDYZ   g_GenerateMips3DCS_sRGB_OddYZ
#define LLGL_IDR_GENERATEMIPS3D_CS_SRGB_ODDXYZ  g_GenerateMips3DCS_sRGB_OddXYZ

#define LLGL_IDR_STREAMOUTPUTDRAWARGS_CS        g_StreamOutputDrawArgsCS


#endif