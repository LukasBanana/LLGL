/*
 * ImageFromURL.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package main

import (
	"log"
	"runtime"
	"unsafe"
	"time"

	"../../../wrapper/Go"
)

import (
	"image"
	_ "image/jpeg" // Register JPEG decoder
	_ "image/png" // Register PNG decoder
	"image/draw"
	"net/http"
)

var g_colorMap llgl.Texture = nil
var g_renderer llgl.RenderSystem = nil
var g_vertexBuffer llgl.Buffer = nil
var g_viewportRatio float32 = 1.0

func LoadImageFromURL(url string) (*image.RGBA, error) {
	response, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	defer response.Body.Close()

	// Decode the image (handles png, jpeg, etc. if imported)
	img, _, err := image.Decode(response.Body)
	if err != nil {
		return nil, err
	}

	// Ensure the image is in RGBA format
	bounds := img.Bounds()
	rgba := image.NewRGBA(bounds)
	draw.Draw(rgba, bounds, img, bounds.Min, draw.Src)

	return rgba, nil
}

func LoadTextureFromURL(url string) (llgl.Texture, int, int) {
	// Create image from URL
	img, err := LoadImageFromURL(url)
	if err != nil {
		log.Fatalf("Failed to load image from URL: %s", err)

		// Create default texture
		return g_renderer.CreateTexture(
			llgl.TextureDescriptor{
				Type:		llgl.TextureTypeTexture2D,
				BindFlags:	llgl.BindSampled | llgl.BindColorAttachment,
				Format:		llgl.FormatRGBA8UNorm,
				Extent:		llgl.Extent3D{ Width: 1, Height: 1, Depth: 1 },
			},
			nil,
		),
		1,
		1
	} else {
		imgWidth := uint32(img.Bounds().Dx())
		imgHeight := uint32(img.Bounds().Dy())

		// Create texture from image
		var pinner runtime.Pinner
		pinner.Pin(&img.Pix[0])
		defer pinner.Unpin()

		return g_renderer.CreateTexture(
			llgl.TextureDescriptor{
				Type:		llgl.TextureTypeTexture2D,
				BindFlags:	llgl.BindSampled | llgl.BindColorAttachment,
				MiscFlags:	llgl.MiscGenerateMips,
				Format:		llgl.FormatRGBA8UNorm,
				Extent:		llgl.Extent3D{ Width: imgWidth, Height: imgHeight, Depth: 1 },
			},
			&llgl.ImageView{
				Format:		llgl.ImageFormatRGBA,
				DataType:	llgl.DataTypeUInt8,
				Data:		unsafe.Pointer(&img.Pix[0]),
				DataSize:	uintptr(len(img.Pix)),
				RowStride:	uint32(img.Stride),
			},
		),
		img.Bounds().Dx(),
		img.Bounds().Dy()
	}
}

func CreateNewColorMapFromURL(url string) {
	if g_colorMap != nil {
		g_renderer.ReleaseTexture(g_colorMap)
		g_colorMap = nil
	}
	var imgWidth int
	var imgHeight int
	g_colorMap, imgWidth, imgHeight = LoadTextureFromURL(url)
	UpdateVertexAspectRatio(0.9, imgWidth, imgHeight)
}

type Vertex struct {
	x float32
	y float32
	u float32
	v float32
}

var g_vertices [4]Vertex

// Updates the coordinates of the input vertices according to the input size
func UpdateVertexAspectRatio(scale float32, imgWidth, imgHeight int) {
	if imgWidth <= 0 || imgHeight <= 0 {
		return
	}

	// Calculate ratios
	imageRatio := g_viewportRatio * (float32(imgWidth) / float32(imgHeight))

	maxWidth := scale
	maxHeight := scale
	
	// Default to full size (-0.5 to 0.5)
	if imageRatio > 1.0 {
		// Landscape: Image is wider than a square. 
		// Keep X at 0.5, shrink Y to fit.
		maxHeight /= imageRatio
	} else if imageRatio < 1.0 {
		// Portrait: Image is taller than a square.
		// Keep Y at 0.5, shrink X to fit.
		maxWidth *= imageRatio
	}

	// Update coordinates (maintaining UVs as they were)
	// Indexing corresponds to your specific vertex order:
	// 0: Left-Top, 1: Left-Bottom, 2: Right-Top, 3: Right-Bottom
	g_vertices[0].x, g_vertices[0].y = -maxWidth, +maxHeight
	g_vertices[1].x, g_vertices[1].y = -maxWidth, -maxHeight
	g_vertices[2].x, g_vertices[2].y = +maxWidth, +maxHeight
	g_vertices[3].x, g_vertices[3].y = +maxWidth, -maxHeight

	// Update vertex buffer
	g_renderer.WriteBuffer(g_vertexBuffer, 0, unsafe.Pointer(&g_vertices), uint64(unsafe.Sizeof(g_vertices)))
}

func main() {

	// Lock the current goroutine to its current OS thread.
	// This ensures all CGO calls happen on the same thread,
	// which is mandatory for the LLGL wrapper.
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	// Load renderer and create swap chain
	g_renderer = llgl.LoadRenderSystem("OpenGL")

	swapChain := g_renderer.CreateSwapChain(llgl.SwapChainDescriptor{ Resolution: llgl.Extent2D{ Width: 800, Height: 600 }, ColorBits: 24, Samples: 8, SwapBuffers: 2 })

	cmdBuffer := g_renderer.CreateCommandBuffer(llgl.CommandBufferDescriptor{ Flags: llgl.CommandBufferImmediateSubmit })

	// Store viewport ratio
	viewportExtent := swapChain.GetResolution()
	g_viewportRatio = float32(viewportExtent.Height) / float32(viewportExtent.Width)

	// Print info about the renderer
	info := g_renderer.GetRendererInfo()
	log.Printf("LLGL Renderer: %s ( %s )", info.RendererName, info.DeviceName)

	// Create vertices
	g_vertices = [4]Vertex{
		{ x: -0.5, y: +0.5, u: 0.0, v: 0.0 }, // Left-Top
		{ x: -0.5, y: -0.5, u: 0.0, v: 1.0 }, // Left-Bottom
		{ x: +0.5, y: +0.5, u: 1.0, v: 0.0 }, // Right-Top
		{ x: +0.5, y: -0.5, u: 1.0, v: 1.0 }, // Right-Bottom
	}

	vertexLayout := []llgl.VertexAttribute{
		{ Name: "position", Format: llgl.FormatRG32Float, Location: 0, Offset: uint32(unsafe.Offsetof(g_vertices[0].x)), Stride: uint32(unsafe.Sizeof(g_vertices[0])) },
		{ Name: "texCoord", Format: llgl.FormatRG32Float, Location: 1, Offset: uint32(unsafe.Offsetof(g_vertices[0].u)), Stride: uint32(unsafe.Sizeof(g_vertices[0])) },
	}
	
	g_vertexBuffer = g_renderer.CreateBuffer(
		llgl.BufferDescriptor{
			Size: uint64(unsafe.Sizeof(g_vertices)),
			BindFlags: llgl.BindVertexBuffer,
			VertexAttribs: vertexLayout,
		},
		unsafe.Pointer(&g_vertices),
	)

	// Create shaders and graphics pipeline
	vertShader := g_renderer.CreateShader(
		llgl.ShaderDescriptor{
			Type: llgl.ShaderTypeVertex,
			Source: `
				#version 330
				in vec2 position;
				in vec2 texCoord;
				out vec2 vTexCoord;
				void main() {
					gl_Position = vec4(position, 0, 1);
					vTexCoord = texCoord;
				}
			`,
			SourceType: llgl.ShaderSourceTypeCodeString,
			Vertex: llgl.VertexShaderAttributes{ InputAttribs: vertexLayout },
		},
	)

	fragShader := g_renderer.CreateShader(
		llgl.ShaderDescriptor{
			Type: llgl.ShaderTypeFragment,
			Source: `
				#version 330
				in vec2 vTexCoord;
				out vec4 outColor;
				uniform sampler2D colorMap;
				void main() {
					outColor = texture(colorMap, vTexCoord);
				}
			`,
			SourceType: llgl.ShaderSourceTypeCodeString,
		},
	)

	psoLayout := g_renderer.CreatePipelineLayout(llgl.PipelineLayoutDescriptor{
		Bindings: []llgl.BindingDescriptor{
			llgl.BindingDescriptor{
				Name: "colorMap",
				Type: llgl.ResourceTypeTexture,
				BindFlags: llgl.BindSampled,
				StageFlags: llgl.StageFragmentStage,
				Slot: llgl.BindingSlot{ Index: 0, Set: 0 },
			},
		},
	})

	pso := g_renderer.CreateGraphicsPipelineState(llgl.GraphicsPipelineDescriptor{
		PipelineLayout: &psoLayout,
		VertexShader: &vertShader,
		FragmentShader: &fragShader,
		PrimitiveTopology: llgl.PrimitiveTopologyTriangleStrip,
		Rasterizer: llgl.RasterizerDescriptor{
			MultiSampleEnabled: true,
		},
		Blend: llgl.BlendDescriptor{
			Targets: [8]llgl.BlendTargetDescriptor{
				0: { ColorMask: llgl.ColorMaskAll },
			},
		},
	})

	if report := pso.GetReport(); report.HasErrors() {
		log.Fatalf("PSO compilation failed:\n%s", report.GetText())
	}

	// Create initial texture
	imgUrl := "https://picsum.photos/200/300"
	CreateNewColorMapFromURL(imgUrl)

	// Setup timer to update the image every few seconds
	interval := 2 * time.Second
	lastTrigger := time.Now()

	// Main loop
	window := llgl.AsWindow(swapChain.GetSurface())
	for llgl.ProcessSurfaceEvents() && !window.HasQuit() {
		// Update texture ever few seconds
		if time.Since(lastTrigger) >= interval {
			CreateNewColorMapFromURL(imgUrl)
			lastTrigger = time.Now()
		}

		// Render frame
		cmdBuffer.Begin()
		cmdBuffer.BeginRenderPass(swapChain)

		resolution := swapChain.GetResolution()
		cmdBuffer.Clear(llgl.ClearColor, llgl.ClearValue{ Color: [4]float32{ 0.1, 0.1, 0.2, 1.0} })
		cmdBuffer.SetPipelineState(pso)
		cmdBuffer.SetViewport(llgl.Viewport{ X: 0.0, Y: 0.0, Width: float32(resolution.Width), Height: float32(resolution.Height), MinDepth: 0.0, MaxDepth: 1.0 })

		cmdBuffer.SetVertexBuffer(g_vertexBuffer)
		cmdBuffer.SetResource(0, g_colorMap)
		cmdBuffer.Draw(4, 0)

		cmdBuffer.EndRenderPass()
		cmdBuffer.End()

		swapChain.Present()
	}

	llgl.UnloadRenderSystem()

}
