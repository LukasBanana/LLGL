# Tessellation

<p align="center"><img src="Example.png"/></p>

## Prerequisites

This tutorial requires that you have already read the [Hello Triangle](../HelloTriangle) tutorial.


## Command Queue

In the last tutorial we created an immediate command buffer, but in this one we will use a regular command buffer that we have to submit explicitly to the command queue:
```cpp
LLGL::CommandQueue* myCmdQueue = myRenderer->GetCommandQueue();
LLGL::CommandBuffer* myCmdBuffer = myRenderer->CreateCommandBuffer();
```
In LLGL, there is only one instance of the command queue, which we can retrieve via `GetCommandQueue`.
After we are done recording draw and compute commands, we have to submit the command buffer to the command queue like so:
```cpp
myCmdBuffer->Begin();
/* Record GPU commands here ... */
myCmdBuffer->End();
myCmdQueue->Submit(*myCmdBuffer);
```


## Shader Resources

In legacy rendering APIs, such as OpenGL, shader resources were bound individually like with `glBindTexture`. Recent extensions allowed to bind multiple textures or uniform buffers at once. With modern rendering APIs there is only the option to bind one or more heaps of resources. This is either called "descriptor heap" (Direct3D 12) or "descriptor set" (Vulkan). In LLGL, this is managed by the `ResourceHeap` interface. But before we can create such a resource heap, we need a pipeline layout that specifies at which binding points the resources in the heap will be bound to a graphics or compute pipeline. This is done with the `PipelineLayout` interface and created as follows:
```cpp
LLGL::PipelineLayoutDescriptor myLayoutDesc;
myLayoutDesc.bindings = {
    LLGL::BindingDescriptor {
        "MyConstantBuffer",
        LLGL::ResourceType::Buffer,
        LLGL::BindFlags::ConstantBuffer,
        (IsMetal()
            ? LLGL::StageFlags::ComputeStage | LLGL::StageFlags::VertexStage
            : LLGL::StageFlags::AllTessStages),
        myConstantBufferBindingPoint
    }
};
LLGL::PipelineLayout* myPipelineLayout = myRenderer->CreatePipelineLayout(myLayoutDesc);
```
We need to specify different shader stages for the Metal backend. Although Metal supports tessellation there are no dedicated tessellation shader stages. They are laid out into a compute kernel and a so called "post-tessellation vertex shader" instead. For the tessellation tutorial, we only need a constant buffer that is bound to the tessellation control and tessellation evaluation shader stages. The descriptor field `bindings` is a container which we can easily initialize with a brace initializer list. The parameter `myConstantBufferBindingPoint` is just an unsigned integer that specifies the binding point. If its value is 3 for instance, the corresponding constant buffer in an HLSL shader could look like this:
```hlsl
cbuffer MyConstantBuffer : register(b3) { /*...*/ }
```
There is a utility function that constructs a pipeline layout descriptor with a single string rather than a list of binding descritpors. Assuming we don't support the Metal API and our binding point `myConstantBufferBindingPoint` has value 0, we can simlpy write this:
```cpp
LLGL::PipelineLayoutDescriptor myLayoutDesc = LLGL::PipelineLayoutDesc("cbuffer(MyConstantBuffer@0):tesc:tese");
```

If we add more elements to the `bindings` container, we have to make sure the order of resources matches with the resource view descriptors we fill the resource heap with. We can either initialize the resource heap will all resources at creation time or fill them in later.
```cpp
LLGL::ResourceHeapDescriptor myResourceHeapDesc;
myResourceHeapDesc.pipelineLayout   = myPipelineLayout;
myResourceHeapDesc.numResourceViews = 1;
LLGL::ResourceHeap* myResourceHeap = myRenderer->CreateResourceHeap(myResourceHeapDesc, { myConstantBuffer });
```
The resource heap needs a reference to the pipeline layout we created. The resources (previously created with `CreateBuffer`, `CreateTexture`, or `CreateSampler`) are specified in the brace initializer list of the `resourceViews` container. The elements from this container are of the type `LLGL::ResourceViewDescriptor` but can be implicitly constructed with a pointer to a resource object. Speaking of which, all resources inherit from the `Resource` interface and these interfaces are: `Buffer`, `Texture`, and `Sampler`. The number of resources in our resource heap must be a multiple of the number of bindings in our pipeline layout. This way we can store multiple sets of resource views in our heap that we can efficiently swap out at runtime. For this tutorial, we only have a single resource in our heap, but for a pipeline layout of 5 binding points for instance, we could create a resource heap with 5, 10, 15, or more resources as long as they are a multiple of 5. If `numResourceViews` is 0, we have to initialize the resource heap at creation time and the number of initial resources determines the size of the heap. This is mostly supported to implicitly construct the `ResourceHeapDescriptor` for convenience which can be done like this:
```cpp
LLGL::ResourceHeap* myResourceHeap = myRenderer->CreateResourceHeap(myPipelineLayout, { myConstantBuffer });
```
Ideally, an LLGL program has one resource heap for each pipeline layout. Having said that, there is no limit on how many resource heaps can be created that share the same pipeline layout.


## Graphics Pipeline

Once we use resource heaps, we also need to specify our pipeline layout for the graphics pipeline where the resources are accessed:
```cpp
LLGL::GraphicsPipelineDescriptor myPipelineDesc;
myPipelineDesc.vertexShader         = myVertexShader;                    // Vertex shader
myPipelineDesc.tessControlShader    = myTessControlShader;               // Tessellation-control shader, aka. "Hull" shader
myPipelineDesc.tessEvaluationShader = myTessEvaluationShader;            // Tessellation-evaluation shader, aka. "Domain" shader
myPipelineDesc.fragmentShader       = myFragmentShader;                  // Fragment shader, aka. "Pixel" shader
myPipelineDesc.pipelineLayout       = myPipelineLayout;                  // Specify our pipeline layout
myPipelineDesc.primitiveTopology    = LLGL::PrimitiveTopology::Patches4; // Input topology: patches with 4 control points
myPipelineDesc.depth.testEnabled    = true;                              // Enable depth test
myPipelineDesc.depth.writeEnabled   = true;                              // Enable depth writing
myPipelineDesc.rasterizer.cullMode  = LLGL::CullMode::Back;              // Enable back-face culling
myPipelineDesc.rasterizer.frontCCW  = true;                              // Front facing polygons: counter-clock-wise (CCW) winding
```
While most rendering APIs provide the tessellation parameters on the shader side, in Metal we need to specify some on the host application side.
The following parameters are ignored by all other backends:
```cpp
// We'll use 32-bit indices
pipelineDesc.tessellation.indexFormat       = LLGL::Format::R32UInt;

// Equivalent to [partitioning("fractional_odd")] in HLSL
pipelineDesc.tessellation.partition         = LLGL::TessellationPartition::FractionalOdd;

// Equivalent to [outputtopology("triangle_ccw")] in HLSL
pipelineDesc.tessellation.outputWindingCCW  = true;
```
Now we create the graphics pipeline state object (PSO):
```cpp
LLGL::PipelineState* myPipeline = myRenderer->CreatePipelineState(myPipelineDesc);
```
There are several parameters besides the pipeline layout that are needed for the tessellation tutorial. This time we use the depth buffer to render a 3D scene and not just a flat triangle. We also enable back-face culling as a minor optimization to omit triangles that are never visible since we render an enclosed object and the interior will be hidden. When tessellation shaders are used in the graphics pipeline, the primitive toplogy must be one of the `LLGL::PrimitiveTopology::Patches1`-`32` enumeration entries. The number specifies the control point count. The maximum number of control points that are supported by the host platform can be determined as shown here:
```cpp
myRenderer->GetRenderingCaps().limits.maxPatchVertices
```


## Rendering

Since we use a depth buffer, we need to clear it just like the color buffer:
```cpp
myCmdBuffer->Clear(LLGL::ClearFlags::ColorDepth);
```
The flag `ColorDepth` is just a shortcut for `LLGL::ClearFlags::Color | LLGL::ClearFlags::Depth`.

Together with the vertex buffer, we also use an index buffer for this tutorial to utilize each vertex for multiple polygons. The index buffer creation works analogous to the vertex buffer and can be bound with the `SetIndexBuffer` function:
```cpp
myCmdBuffer->SetVertexBuffer(*myVertexBuffer);
myCmdBuffer->SetIndexBuffer(*myIndexBuffer);
```

Next, we bind the resource heap to the graphics pipeline:
```cpp
myCmdBuffer->SetResourceHeap(*myResourceHeap);
```
Last thing to mention which is different to the previous tutorial: we use the `DrawIndexed` function instead of `Draw`. Otherwise, the index buffer would be pointless for the draw command. If we want to render a tessellated cube, we need 6 patches each of which has 4 control points (since we used `Patches4` as topology). This means we have to generate 24 indices (24=6*4):
```cpp
myCmdBuffer->DrawIndexed(24, 0);
```


That's all folks :-)


