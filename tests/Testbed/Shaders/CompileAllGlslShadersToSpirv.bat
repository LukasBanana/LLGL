@echo off

echo ####### TriangleMesh #######
glslangValidator -V -DENABLE_SPIRV=1 -o TriangleMesh.450core.vert.spv TriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o TriangleMesh.450core.frag.spv TriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -DENABLE_TEXTURING=1 -o TriangleMesh.Textured.450core.vert.spv TriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -DENABLE_TEXTURING=1 -o TriangleMesh.Textured.450core.frag.spv TriangleMesh.450core.frag
echo DONE

pause
