
REM Tutorial01_HelloTriangle
echo ####### Tutorial01_HelloTriangle #######
glslang -V -S vert -o Tutorial01_HelloTriangle/vertex.450core.spv Tutorial01_HelloTriangle/vertex.450core.glsl
glslang -V -S frag -o Tutorial01_HelloTriangle/fragment.450core.spv Tutorial01_HelloTriangle/fragment.450core.glsl

REM Tutorial02_Tessellation
echo ####### Tutorial02_Tessellation #######
glslang -V -S vert -o Tutorial02_Tessellation/vertex.450core.spv Tutorial02_Tessellation/vertex.450core.glsl
glslang -V -S frag -o Tutorial02_Tessellation/fragment.450core.spv Tutorial02_Tessellation/fragment.450core.glsl
glslang -V -S tesc -o Tutorial02_Tessellation/tesscontrol.450core.spv Tutorial02_Tessellation/tesscontrol.450core.glsl
glslang -V -S tese -o Tutorial02_Tessellation/tesseval.450core.spv Tutorial02_Tessellation/tesseval.450core.glsl

REM Tutorial03_Texturing
echo ####### Tutorial03_Texturing #######
glslang -V -S vert -o Tutorial03_Texturing/vertex.450core.spv Tutorial03_Texturing/vertex.450core.glsl
glslang -V -S frag -o Tutorial03_Texturing/fragment.450core.spv Tutorial03_Texturing/fragment.450core.glsl

REM Tutorial05_RenderTarget
echo ####### Tutorial05_RenderTarget #######
glslang -V -S vert -o Tutorial05_RenderTarget/vertex.450core.spv Tutorial05_RenderTarget/vertex.450core.glsl
glslang -V -S frag -o Tutorial05_RenderTarget/fragment.450core.spv Tutorial05_RenderTarget/fragment.450core.glsl

REM Tutorial06_MultiContext
echo ####### Tutorial06_MultiContext #######
glslang -V -S vert -o Tutorial06_MultiContext/vertex.450core.spv Tutorial06_MultiContext/vertex.450core.glsl
glslang -V -S frag -o Tutorial06_MultiContext/fragment.450core.spv Tutorial06_MultiContext/fragment.450core.glsl
glslang -V -S geom -o Tutorial06_MultiContext/geometry.450core.spv Tutorial06_MultiContext/geometry.450core.glsl

REM Tutorial07_Array
echo ####### Tutorial07_Array #######
glslang -V -S vert -o Tutorial07_Array/vertex.450core.spv Tutorial07_Array/vertex.450core.glsl
glslang -V -S frag -o Tutorial07_Array/fragment.450core.spv Tutorial07_Array/fragment.450core.glsl

REM Tutorial08_Compute
echo ####### Tutorial08_Compute #######
glslang -V -S comp -o Tutorial08_Compute/compute.spv Tutorial08_Compute/compute.glsl

REM Tutorial10_Instancing
echo ####### Tutorial10_Instancing #######
glslang -V -S vert -o Tutorial10_Instancing/vertex.450core.spv Tutorial10_Instancing/vertex.450core.glsl
glslang -V -S frag -o Tutorial10_Instancing/fragment.450core.spv Tutorial10_Instancing/fragment.450core.glsl

REM Tutorial11_PostProcessing
echo ####### Tutorial11_PostProcessing #######
glslang -V -S vert -o Tutorial11_PostProcessing/scene.vertex.spv Tutorial11_PostProcessing/scene.vertex.450core.glsl
glslang -V -S vert -o Tutorial11_PostProcessing/postprocess.vertex.spv Tutorial11_PostProcessing/postprocess.vertex.450core.glsl
glslang -V -S frag -o Tutorial11_PostProcessing/scene.fragment.spv Tutorial11_PostProcessing/scene.fragment.450core.glsl
glslang -V -S frag -o Tutorial11_PostProcessing/final.fragment.spv Tutorial11_PostProcessing/final.fragment.450core.glsl
glslang -V -S frag -o Tutorial11_PostProcessing/blur.fragment.spv Tutorial11_PostProcessing/blur.fragment.450core.glsl

pause