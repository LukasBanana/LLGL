/*
 * Texture.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public sealed class Texture : Resource
    {
        internal NativeLLGL.Texture Native { get; private set; }

        internal override NativeLLGL.Resource NativeBase
        {
            get
            {
                unsafe
                {
                    return new NativeLLGL.Resource() { ptr = Native.ptr };
                }
            }
        }

        internal Texture(NativeLLGL.Texture native, string debugName = null)
        {
            Native = native;
            InitializeDebugName(debugName);
        }

        ~Texture()
        {
            NativeLLGL.ReleaseTexture(Native);
        }

        public TextureType TextureType
        {
            get
            {
                return NativeLLGL.GetTextureType(Native);
            }
        }

        public BindFlags BindFlags
        {
            get
            {
                return (BindFlags)NativeLLGL.GetTextureBindFlags(Native);
            }
        }

        public TextureDescriptor Desc
        {
            get
            {
                var nativeDesc = new NativeLLGL.TextureDescriptor();
                NativeLLGL.GetTextureDesc(Native, ref nativeDesc);
                return new TextureDescriptor(nativeDesc);
            }
        }

        public Format Format
        {
            get
            {
                return NativeLLGL.GetTextureFormat(Native);
            }
        }

        public Extent3D GetMipExtent(int mipLevel)
        {
            var mipExtent = new Extent3D();
            NativeLLGL.GetTextureMipExtent(Native, mipLevel, ref mipExtent);
            return mipExtent;
        }

        public SubresourceFootprint GetSubresourceFootprint(int mipLevel)
        {
            var footprint = new SubresourceFootprint();
            NativeLLGL.GetTextureSubresourceFootprint(Native, mipLevel, ref footprint);
            return footprint;
        }
    }
}




// ================================================================================
