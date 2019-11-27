
echo ####### HelloTriangle #######
glslangValidator -V -S vert -o HelloTriangle/Example.450core.vert.spv HelloTriangle/Example.450core.vert
glslangValidator -V -S frag -o HelloTriangle/Example.450core.frag.spv HelloTriangle/Example.450core.frag

echo ####### Tessellation #######
glslangValidator -V -S vert -o Tessellation/Example.450core.vert.spv Tessellation/Example.450core.vert
glslangValidator -V -S tesc -o Tessellation/Example.450core.tesc.spv Tessellation/Example.450core.tesc
glslangValidator -V -S tese -o Tessellation/Example.450core.tese.spv Tessellation/Example.450core.tese
glslangValidator -V -S frag -o Tessellation/Example.450core.frag.spv Tessellation/Example.450core.frag

echo ####### Texturing #######
glslangValidator -V -S vert -o Texturing/Example.450core.vert.spv Texturing/Example.450core.vert
glslangValidator -V -S frag -o Texturing/Example.450core.frag.spv Texturing/Example.450core.frag

echo ####### Queries #######
glslangValidator -V -S vert -o Queries/Example.450core.vert.spv Queries/Example.450core.vert
glslangValidator -V -S frag -o Queries/Example.450core.frag.spv Queries/Example.450core.frag

echo ####### RenderTarget #######
glslangValidator -V -S vert -o RenderTarget/Example.450core.vert.spv RenderTarget/Example.450core.vert
glslangValidator -V -S frag -o RenderTarget/Example.450core.frag.spv RenderTarget/Example.450core.frag

echo ####### ShadowMapping #######
glslangValidator -V -S vert -o ShadowMapping/ShadowMap.450core.vert.spv ShadowMapping/ShadowMap.450core.vert
glslangValidator -V -S vert -o ShadowMapping/Scene.450core.vert.spv ShadowMapping/Scene.450core.vert
glslangValidator -V -S frag -o ShadowMapping/Scene.450core.frag.spv ShadowMapping/Scene.450core.frag

echo ####### MultiContext #######
glslangValidator -V -S vert -o MultiContext/Example.450core.vert.spv MultiContext/Example.450core.vert
glslangValidator -V -S geom -o MultiContext/Example.450core.geom.spv MultiContext/Example.450core.geom
glslangValidator -V -S frag -o MultiContext/Example.450core.frag.spv MultiContext/Example.450core.frag

echo ####### MultiRenderer #######
glslangValidator -V -S vert -o MultiRenderer/Example.450core.vert.spv MultiRenderer/Example.450core.vert
glslangValidator -V -S frag -o MultiRenderer/Example.450core.frag.spv MultiRenderer/Example.450core.frag

echo ####### MultiThreading #######
glslangValidator -V -S vert -o MultiThreading/Example.450core.vert.spv MultiThreading/Example.450core.vert
glslangValidator -V -S frag -o MultiThreading/Example.450core.frag.spv MultiThreading/Example.450core.frag

echo ####### BufferArray #######
glslangValidator -V -S vert -o BufferArray/Example.450core.vert.spv BufferArray/Example.450core.vert
glslangValidator -V -S frag -o BufferArray/Example.450core.frag.spv BufferArray/Example.450core.frag

echo ####### ComputeShader #######
glslangValidator -V -S comp -o ComputeShader/Example.comp.spv ComputeShader/Example.comp
glslangValidator -V -S vert -o ComputeShader/Example.vert.spv ComputeShader/Example.vert
glslangValidator -V -S frag -o ComputeShader/Example.frag.spv ComputeShader/Example.frag

echo ####### Instancing #######
glslangValidator -V -S vert -o Instancing/Example.450core.vert.spv Instancing/Example.450core.vert
glslangValidator -V -S frag -o Instancing/Example.450core.frag.spv Instancing/Example.450core.frag

echo ####### PostProcessing #######
glslangValidator -V -S vert -o PostProcessing/Scene.450core.vert.spv PostProcessing/Scene.450core.vert
glslangValidator -V -S frag -o PostProcessing/Scene.450core.frag.spv PostProcessing/Scene.450core.frag
glslangValidator -V -S vert -o PostProcessing/PostProcess.450core.vert.spv PostProcessing/PostProcess.450core.vert
glslangValidator -V -S frag -o PostProcessing/Final.450core.frag.spv PostProcessing/Final.450core.frag
glslangValidator -V -S frag -o PostProcessing/Blur.450core.frag.spv PostProcessing/Blur.450core.frag

echo ####### StencilBuffer #######
glslangValidator -V -S vert -o StencilBuffer/Stencil.450core.vert.spv StencilBuffer/Stencil.450core.vert
glslangValidator -V -S vert -o StencilBuffer/Scene.450core.vert.spv StencilBuffer/Scene.450core.vert
glslangValidator -V -S frag -o StencilBuffer/Scene.450core.frag.spv StencilBuffer/Scene.450core.frag

echo ####### StreamOutput #######
glslangValidator -V -S vert -o StreamOutput/Example.450core.vert.spv StreamOutput/Example.450core.vert
glslangValidator -V -S geom -o StreamOutput/Example.450core.geom.spv StreamOutput/Example.450core.geom
glslangValidator -V -S frag -o StreamOutput/Example.450core.frag.spv StreamOutput/Example.450core.frag

echo ####### Animation #######
glslangValidator -V -S vert -o Animation/Example.450core.vert.spv Animation/Example.450core.vert
glslangValidator -V -S frag -o Animation/Example.450core.frag.spv Animation/Example.450core.frag

echo ####### VolumeRendering #######
glslangValidator -V -S vert -o VolumeRendering/Example.450core.vert.spv VolumeRendering/Example.450core.vert
glslangValidator -V -S frag -o VolumeRendering/Example.450core.frag.spv VolumeRendering/Example.450core.frag

echo ####### ClothPhysics #######
glslangValidator -V -S vert -o ClothPhysics/Example.VS.450core.vert.spv ClothPhysics/Example.VS.450core.vert
glslangValidator -V -S frag -o ClothPhysics/Example.PS.450core.frag.spv ClothPhysics/Example.PS.450core.frag
glslangValidator -V -S comp -o ClothPhysics/Example.CSForces.450core.comp.spv ClothPhysics/Example.CSForces.450core.comp
glslangValidator -V -S comp -o ClothPhysics/Example.CSStretchConstraints.450core.comp.spv ClothPhysics/Example.CSStretchConstraints.450core.comp
glslangValidator -V -S comp -o ClothPhysics/Example.CSRelaxation.450core.comp.spv ClothPhysics/Example.CSRelaxation.450core.comp

echo ####### UnorderedAccess #######
glslangValidator -V -S vert -o UnorderedAccess/Example.450core.vert.spv UnorderedAccess/Example.450core.vert
glslangValidator -V -S frag -o UnorderedAccess/Example.450core.frag.spv UnorderedAccess/Example.450core.frag
glslangValidator -V -S comp -o UnorderedAccess/Example.450core.comp.spv UnorderedAccess/Example.450core.comp

echo ####### Mapping #######
glslangValidator -V -S vert -o Mapping/Example.450core.vert.spv Mapping/Example.450core.vert
glslangValidator -V -S frag -o Mapping/Example.450core.frag.spv Mapping/Example.450core.frag

pause