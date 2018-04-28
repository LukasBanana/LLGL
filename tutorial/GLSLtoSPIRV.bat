
REM Tutorial01_HelloTriangle
glslang -V -S vert -o Tutorial01_HelloTriangle/vertex.450core.spv Tutorial01_HelloTriangle/vertex.450core.glsl
glslang -V -S frag -o Tutorial01_HelloTriangle/fragment.450core.spv Tutorial01_HelloTriangle/fragment.450core.glsl

REM Tutorial02_Tessellation
glslang -V -S vert -o Tutorial02_Tessellation/vertex.450core.spv Tutorial02_Tessellation/vertex.450core.glsl
glslang -V -S frag -o Tutorial02_Tessellation/fragment.450core.spv Tutorial02_Tessellation/fragment.450core.glsl
glslang -V -S tesc -o Tutorial02_Tessellation/tesscontrol.450core.spv Tutorial02_Tessellation/tesscontrol.450core.glsl
glslang -V -S tese -o Tutorial02_Tessellation/tesseval.450core.spv Tutorial02_Tessellation/tesseval.450core.glsl

REM Tutorial08_Compute
glslang -V -S comp -o Tutorial08_Compute/compute.spv Tutorial08_Compute/compute.glsl

pause