/*
 * Direct3D11.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DIRECT3D11_H
#define LLGL_DIRECT3D11_H


#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
#   include <d3d11_3.h>
#elif LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
#   include <d3d11_2.h>
#elif LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
#   include <d3d11_1.h>
#else
#   include <d3d11.h>
#endif


#endif



// ================================================================================
