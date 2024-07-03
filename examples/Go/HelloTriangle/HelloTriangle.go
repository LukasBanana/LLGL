/*
 * HelloTriangle.go
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

package main

import (
	"log"
	"unsafe"

	"../../../wrapper/Go"
)

func main() {

	// Load renderer and create swap chain
	renderer := llgl.LoadRenderSystem("OpenGL")

	swapChain := renderer.CreateSwapChain(llgl.SwapChainDescriptor{ Resolution: llgl.Extent2D{ Width: 800, Height: 600 }, ColorBits: 24, Samples: 8, SwapBuffers: 2 })

	cmdBuffer := renderer.CreateCommandBuffer(llgl.CommandBufferDescriptor{ Flags: llgl.CommandBufferImmediateSubmit })

	// Print info about the renderer
	info := renderer.GetRendererInfo()
	log.Printf("LLGL Renderer: %s ( %s )", info.RendererName, info.DeviceName)

	// Create vertices
	type Vertex struct {
		x float32
		y float32
		color uint32
	}

	vertices := [3]Vertex{
		Vertex{ x:  0.0, y: +0.5, color: 0xFF0000FF }, // Red
		Vertex{ x: +0.5, y: -0.5, color: 0xFF00FF00 }, // Green
		Vertex{ x: -0.5, y: -0.5, color: 0xFFFF0000 }, // Blue
	}

	vertexLayout := []llgl.VertexAttribute{
		llgl.VertexAttribute{ Name: "position", Format: llgl.FormatRG32Float,  Location: 0, Offset: uint32(unsafe.Offsetof(vertices[0].x    )), Stride: uint32(unsafe.Sizeof(vertices[0])) },
		llgl.VertexAttribute{ Name: "color",    Format: llgl.FormatRGBA8UNorm, Location: 1, Offset: uint32(unsafe.Offsetof(vertices[0].color)), Stride: uint32(unsafe.Sizeof(vertices[0])) },
	}
	
	vertexBuffer := renderer.CreateBuffer(
		llgl.BufferDescriptor{
			Size: uint64(unsafe.Sizeof(vertices)),
			BindFlags: llgl.BindVertexBuffer,
			VertexAttribs: vertexLayout,
		},
		unsafe.Pointer(&vertices),
	)

	// Create shaders and graphics pipeline
	vertShader := renderer.CreateShader(
		llgl.ShaderDescriptor{
			Type: llgl.ShaderTypeVertex,
			Source: `
				#version 330
				in vec2 position;
				in vec4 color;
				out vec4 vColor;
				void main() {
					gl_Position = vec4(position, 0, 1);
					vColor = color;
				}
			`,
			SourceType: llgl.ShaderSourceTypeCodeString,
			Vertex: llgl.VertexShaderAttributes{ InputAttribs: vertexLayout },
		},
	)

	fragShader := renderer.CreateShader(
		llgl.ShaderDescriptor{
			Type: llgl.ShaderTypeFragment,
			Source: `
				#version 330
				in vec4 vColor;
				out vec4 outColor;
				void main() {
					outColor = vColor;
				}
			`,
			SourceType: llgl.ShaderSourceTypeCodeString,
		},
	)

	pso := renderer.CreateGraphicsPipelineState(llgl.GraphicsPipelineDescriptor{
		VertexShader: &vertShader,
		FragmentShader: &fragShader,
		PrimitiveTopology: llgl.PrimitiveTopologyTriangleList,
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

	// Main loop
	for llgl.ProcessSurfaceEvents() {
		cmdBuffer.Begin()
		cmdBuffer.BeginRenderPass(swapChain)

		resolution := swapChain.GetResolution()
		cmdBuffer.Clear(llgl.ClearColor, llgl.ClearValue{ Color: [4]float32{ 0.1, 0.1, 0.2, 1.0} })
		cmdBuffer.SetPipelineState(pso)
		cmdBuffer.SetViewport(llgl.Viewport{ X: 0.0, Y: 0.0, Width: float32(resolution.Width), Height: float32(resolution.Height), MinDepth: 0.0, MaxDepth: 1.0 })

		cmdBuffer.SetVertexBuffer(vertexBuffer)
		cmdBuffer.Draw(3, 0)

		cmdBuffer.EndRenderPass()
		cmdBuffer.End()

		swapChain.Present()
	}

	llgl.UnloadRenderSystem()

}
