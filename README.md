Low Level Graphics Library (LLGL)
=================================

License
-------

[3-Clause BSD License](https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt)


Documentation
-------------

- [Getting Started with LLGL](https://github.com/LukasBanana/LLGL/blob/master/docu/GettingStarted/Getting%20Started%20with%20LLGL.pdf) (PDF)
- [LLGL Reference Manual](https://github.com/LukasBanana/LLGL/blob/master/docu/refman.pdf) (PDF)


Progress
--------

- **Version**: 1.00 Alpha

| Renderer | Progress | Remarks |
|----------|:--------:|---------|
| OpenGL | ~80% | |
| Direct3D 11 | ~60% | ToDo: Storage buffers, Render targets |
| Direct3D 12 | ~5% | Experimental state |
| Vulkan | 0% | Not started yet |


Thin Abstraction Layer
----------------------

```cpp
// Interface:
RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex);

// OpenGL Implementation:
void GLRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
	glDrawElements(
		renderState_.drawMode,
		static_cast<GLsizei>(numVertices),
		renderState_.indexBufferDataType,
		(reinterpret_cast<const GLvoid*>(firstIndex * renderState_.indexBufferStride))
	);
}

// Direct3D 11 Implementation
void D3D11RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
	context_->DrawIndexed(numVertices, firstIndex, 0);
}

// Direct3D 12 Implementation
void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
	commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
}

// Vulkan Implementation
void VKRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
	vkCmdDrawIndexed(commandBuffer_, numVertices, 1, firstIndex, 0, 0);
}
```


Tutorials and Examples
----------------------

This repository contains several tutorials and examples which show how to use LLGL.
Here are a few with a screenshot:

### Tutorial 01: Hello Triangle

Getting started tutorial where a single multi-colored triangle is rendered.

<p align="center"><img src="tutorial/Tutorial01_HelloTriangle.png" width="512" height="384"/></p>

### Tutorial 02: Tessellation

Simple tessellation example without any textures.

![alt text](https://github.com/LukasBanana/LLGL/blob/master/tutorial/Tutorial02_Tessellation.png)

### Tutorial 05: Render Target

![alt text](https://github.com/LukasBanana/LLGL/blob/master/tutorial/Tutorial05_RenderTarget.png)

### Tutorial 06: Multi Context

Multi-context tutorial shows the following rendering techniques:
multiple render contexts (one window each), rendering simultaneously into multiple viewports, geometry shader.

![alt text](https://github.com/LukasBanana/LLGL/blob/master/tutorial/Tutorial06_MultiContext.png)


