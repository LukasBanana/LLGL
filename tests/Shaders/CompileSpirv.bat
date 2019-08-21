glslangValidator -V -S vert -o Triangle.vert.spv Triangle.vert
glslangValidator -V -S frag -o Triangle.frag.spv Triangle.frag
glslangValidator -V -S comp -o SpirvReflectTest.comp.spv SpirvReflectTest.comp
pause