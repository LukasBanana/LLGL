/*
 * TessControlShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TESS_CONTROL_SHADER_H__
#define __LLGL_TESS_CONTROL_SHADER_H__


#include "Export.h"


namespace LLGL
{


//! Tessellation-control shader (also "Hull Shader") interface.
class LLGL_EXPORT TessControlShader
{

    public:

        virtual ~TessControlShader()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
