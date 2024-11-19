# Tutorials and Examples

This repository contains several tutorials and examples which show how to use LLGL. Note that you need to set the **working directory** to `<Your-LLGL-Repository>/examples/Cpp/<Example>` in order to get your examples running. For the first example, this could be `/Users/JohnDoe/LLGL/examples/Cpp/HelloTriangle` or `C:\Users\JohnDoe\LLGL\examples\Cpp\HelloTriangle` for instance.

## Tutorials

### [Hello Triangle](HelloTriangle)

Getting started tutorial where a single multi-colored triangle is rendered.

<p align="center"><img src="HelloTriangle/Example.png" style="width:400px;height:auto;"/></p>


### [Tessellation](Tessellation)

Hardware tessellation for rendering highly detailed geometry.

<p align="center"><img src="Tessellation/Example.png" style="width:400px;height:auto;"/></p>


### [ImGui](../../docu/Tutorials/ImGui)

Tutorial how to integrate ImGui into an LLGL project for UI rendering and event handling.

<p align="center"><img src="../../docu/Tutorials/ImGui/LLGL_ImGui_Thumbnail.png" style="width:400px;height:auto;"/></p>


## Examples

### [Texturing](Texturing)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_Texturing/index.html)

Simple texturing and sampler state example.

<p align="center"><img src="Texturing/Example.png" style="width:400px;height:auto;"/></p>


### [Fonts](Fonts)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_Fonts/index.html)

Example how to render fonts efficiently in batched draw calls.

<p align="center"><img src="Fonts/Example.png" style="width:400px;height:auto;"/></p>


### [Queries](Queries)

Shows how to use Query objects and conditional rendering for occlusion culling.

<p align="center"><img src="Queries/Example.png" style="width:400px;height:auto;"/></p>


### [Render Target](RenderTarget)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_RenderTarget/index.html)

Simple render target example with optional multi-sample texture (Texture2DMS/ sampler2DMS).

<p align="center"><img src="RenderTarget/Example.png" style="width:400px;height:auto;"/></p>


### [Multi Context](MultiContext)

Shows the following rendering techniques: multiple render contexts (one window each), rendering simultaneously into multiple viewports, geometry shader.

<p align="center"><img src="MultiContext/Example.png" style="width:752px;height:auto;"/></p>


### [Indirect Draw](IndirectDraw)

Small example with a compute shader and a storage buffer for the indirect draw command.

<p align="center"><img src="IndirectDraw/Example.png" style="width:400px;height:auto;"/></p>


### [Instancing](Instancing)

Practical example of hardware instancing by rendering tens of thousands of different textured plants instances.

<p align="center"><img src="Instancing/Example.png" style="width:400px;height:auto;"/></p>


### [Post-Processing](PostProcessing)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_PostProcessing/index.html)

Practical example of a glow effect with post-processing and the usage of several shaders, render targets and graphics pipelines.

<p align="center"><img src="PostProcessing/Example.png" style="width:400px;height:auto;"/></p>


### [Multi Renderer](MultiRenderer)

Experimental example of using multiple renderers at once (only supported on Win32 platform).

<p align="center"><img src="MultiRenderer/Example.png" style="width:400px;height:auto;"/></p>


### [Shadow Mapping](ShadowMapping)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_ShadowMapping/index.html)

Practical example of standard shadow-mapping technique.

<p align="center"><img src="ShadowMapping/Example.png" style="width:400px;height:auto;"/></p>


### [Stencil Buffer](StencilBuffer)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_StencilBuffer/index.html)

Rendering a portal using the stencil buffer.

<p align="center"><img src="StencilBuffer/Example.png" style="width:400px;height:auto;"/></p>


### [Animation](Animation)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_Animation/index.html)

Small animation example with orthogonal projection.

<p align="center"><img src="Animation/Example.gif" style="width:400px;height:auto;"/></p>


### [Volume Rendering](VolumeRendering)

Example of generating perlin noise into 3D texture with a glitter effect and volume rendering.

<p align="center"><img src="VolumeRendering/Example.png" style="width:400px;height:auto;"/></p>


### [Cloth Physics](ClothPhysics)

Practical example of multiple compute shaders for position based dynamics.

<p align="center"><img src="ClothPhysics/Example.gif" style="width:400px;height:auto;"/></p>


### [HelloGame](HelloGame)
##### Try it with [WebGL](https://lukasbanana.github.io/LLGL/docu/WebPage/Example_HelloGame/index.html)

Example of a small puzzle game. Easy to add new levels via text files.

<p align="center"><img src="HelloGame/HelloGame.gif" style="width:400px;height:auto;"/></p>



