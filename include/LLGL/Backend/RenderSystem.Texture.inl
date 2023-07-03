/*
 * RenderSystem.Texture.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Textures ----- */

virtual LLGL::Texture* CreateTexture(
    const LLGL::TextureDescriptor&  textureDesc,
    const LLGL::SrcImageDescriptor* imageDesc       = nullptr
) override final;

virtual void Release(
    LLGL::Texture&                  texture
) override final;

virtual void WriteTexture(
    LLGL::Texture&                  texture,
    const LLGL::TextureRegion&      textureRegion,
    const LLGL::SrcImageDescriptor& imageDesc
) override final;

virtual void ReadTexture(
    LLGL::Texture&                  texture,
    const LLGL::TextureRegion&      textureRegion,
    const LLGL::DstImageDescriptor& imageDesc
) override final;



// ================================================================================
