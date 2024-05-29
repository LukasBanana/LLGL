@echo off

glslangValidator -V -DENABLE_SPIRV=1 -o TriangleMesh.450core.vert.spv TriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o TriangleMesh.450core.frag.spv TriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -DENABLE_TEXTURING=1 -o TriangleMesh.Textured.450core.vert.spv TriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -DENABLE_TEXTURING=1 -o TriangleMesh.Textured.450core.frag.spv TriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o UnprojectedMesh.450core.vert.spv UnprojectedMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o UnprojectedMesh.450core.frag.spv UnprojectedMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o DualSourceBlending.450core.vert.spv DualSourceBlending.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o DualSourceBlending.450core.frag.spv DualSourceBlending.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o DynamicTriangleMesh.450core.vert.spv DynamicTriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o DynamicTriangleMesh.450core.frag.spv DynamicTriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ShadowMapping.VShadow.450core.vert.spv ShadowMapping.VShadow.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ShadowMapping.VScene.450core.vert.spv ShadowMapping.VScene.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ShadowMapping.PScene.450core.frag.spv ShadowMapping.PScene.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o SemanticErrors.PSMain.450core.frag.spv SemanticErrors.PSMain.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceBinding.450core.comp.spv ResourceBinding.450core.comp
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceBinding.450core.vert.spv ResourceBinding.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceBinding.450core.frag.spv ResourceBinding.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ClearScreen.450core.vert.spv ClearScreen.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ClearScreen.450core.frag.spv ClearScreen.450core.frag

echo DONE
pause
