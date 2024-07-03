/*
 * Texture.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package llgl

// #cgo CFLAGS: -I ../../include
// #include <LLGL-C/LLGL.h>
import "C"

type Texture interface {
	Resource
	GetType() TextureType
	GetBindFlags() uint
	GetDesc() TextureDescriptor
	GetFormat() Format
	GetMipExtent(mipLevel uint32) Extent3D
	GetSubresourceFootprint(mipLevel uint32) SubresourceFootprint
}

type textureImpl struct {
	native C.LLGLTexture
}

func (self textureImpl) getNativeResource() C.LLGLResource {
	return C.LLGLResource(self.native)
}

func (self textureImpl) SetDebugName(name string) {
	setRenderSystemChildDebugName(C.LLGLRenderSystemChild(self.native), name)
}

func (self textureImpl) GetResourceType() ResourceType {
	return ResourceType(C.llglGetResourceType(C.LLGLResource(self.native)))
}

func (self textureImpl) GetType() TextureType {
	return TextureType(C.llglGetTextureType(self.native))
}

func (self textureImpl) GetBindFlags() uint {
	return uint(C.llglGetTextureBindFlags(self.native))
}

func (self textureImpl) GetDesc() TextureDescriptor {
	var nativeDesc C.LLGLTextureDescriptor
	C.llglGetTextureDesc(self.native, &nativeDesc)
	return TextureDescriptor{
		Type:	TextureType(nativeDesc._type),
		BindFlags:		uint(nativeDesc.bindFlags),
		Format:			Format(nativeDesc.format),
		Extent:			Extent3D{
							Width:	uint32(nativeDesc.extent.width),
							Height:	uint32(nativeDesc.extent.height),
							Depth:	uint32(nativeDesc.extent.depth) },
		ArrayLayers:	uint32(nativeDesc.arrayLayers),
		MipLevels:		uint32(nativeDesc.mipLevels),
		Samples:		uint32(nativeDesc.samples),
	}
}

func (self textureImpl) GetFormat() Format {
	return Format(C.llglGetTextureFormat(self.native))
}

func (self textureImpl) GetMipExtent(mipLevel uint32) Extent3D {
	var nativeExtent C.LLGLExtent3D
	C.llglGetTextureMipExtent(self.native, C.uint32_t(mipLevel), &nativeExtent)
	return Extent3D{
		uint32(nativeExtent.width),
		uint32(nativeExtent.height),
		uint32(nativeExtent.depth),
	}
}

func (self textureImpl) GetSubresourceFootprint(mipLevel uint32) SubresourceFootprint {
	var nativeFootprint C.LLGLSubresourceFootprint
	C.llglGetTextureSubresourceFootprint(self.native, C.uint32_t(mipLevel), &nativeFootprint)
	return SubresourceFootprint{
		Size:			uint64(nativeFootprint.size),
		RowAlignment:	uint32(nativeFootprint.rowAlignment),
		RowSize:		uint32(nativeFootprint.rowSize),
		RowStride:		uint32(nativeFootprint.rowStride),
		LayerSize:		uint32(nativeFootprint.layerSize),
		LayerStride:	uint32(nativeFootprint.layerStride),
	}
}



// ================================================================================
