/*
 * D3D11TextureArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11TextureArray.h"
#include "D3D11Texture.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


D3D11TextureArray::D3D11TextureArray(std::uint32_t numTextures, Texture* const * textureArray)
{
    /* Store the pointer of each SRV inside the array */
    resourceViews_.reserve(numTextures);
    while (auto next = NextArrayResource<D3D11Texture>(numTextures, textureArray))
        resourceViews_.push_back(next->GetSRV());
}


} // /namespace LLGL



// ================================================================================
