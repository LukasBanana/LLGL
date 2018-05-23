/*
 * TextureArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_ARRAY_H
#define LLGL_TEXTURE_ARRAY_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\brief Texture container interface.
\todo Maybe rename this to "TextureHeap".
\see RenderSystem::CreateTextureArray
*/
class LLGL_EXPORT TextureArray : public RenderSystemChild { };


} // /namespace LLGL


#endif



// ================================================================================
