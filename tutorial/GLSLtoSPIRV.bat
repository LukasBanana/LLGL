
REM Tutorial01_HelloTriangle
glslang -V -S vert -o Tutorial01_HelloTriangle/vertex.450core.spv Tutorial01_HelloTriangle/vertex.450core.glsl
glslang -V -S frag -o Tutorial01_HelloTriangle/fragment.450core.spv Tutorial01_HelloTriangle/fragment.450core.glsl

REM Tutorial08_Compute
glslang -V -S comp -o Tutorial08_Compute/compute.spv Tutorial08_Compute/compute.glsl

pause