/*
 * Direct3D11.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
