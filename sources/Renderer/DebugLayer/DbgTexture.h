/*
 * DbgTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_TEXTURE_H
#define LLGL_DBG_TEXTURE_H


#include <LLGL/Texture.h>
#include <vector>
#include <string>


namespace LLGL
{


class DbgTexture final : public Texture
{

    public:

        void SetName(const char* name) override;

        Extent3D QueryMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

    public:

        DbgTexture(Texture& instance, const TextureDescriptor& desc);
        //DbgTexture(Texture& instance, DbgTexture* sharedTexture, const TextureViewDescriptor& desc);

    public:

        Texture&                    instance;
        const TextureDescriptor     desc;
        TextureViewDescriptor       viewDesc;
        std::uint32_t               mipLevels           = 1;
        std::string                 label;

        DbgTexture*                 sharedTexture       = nullptr;  // Reference to the shared texture (only for texture-views)
        std::vector<DbgTexture*>    sharedTextureViews;             // List of texture views that share the image data with this texture

};


} // /namespace LLGL


#endif



// ================================================================================
