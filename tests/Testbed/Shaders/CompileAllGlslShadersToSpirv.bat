@echo off

glslangValidator -V -DENABLE_SPIRV=1 -o TriangleMesh\TriangleMesh.450core.vert.spv TriangleMesh\TriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o TriangleMesh\TriangleMesh.450core.frag.spv TriangleMesh\TriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -DENABLE_TEXTURING=1 -o TriangleMesh\TriangleMesh.Textured.450core.vert.spv TriangleMesh\TriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -DENABLE_TEXTURING=1 -o TriangleMesh\TriangleMesh.Textured.450core.frag.spv TriangleMesh\TriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o UnprojectedMesh\UnprojectedMesh.450core.vert.spv UnprojectedMesh\UnprojectedMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o UnprojectedMesh\UnprojectedMesh.450core.frag.spv UnprojectedMesh\UnprojectedMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o DualSourceBlending\DualSourceBlending.450core.vert.spv DualSourceBlending\DualSourceBlending.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o DualSourceBlending\DualSourceBlending.450core.frag.spv DualSourceBlending\DualSourceBlending.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o DynamicTriangleMesh\DynamicTriangleMesh.450core.vert.spv DynamicTriangleMesh\DynamicTriangleMesh.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o DynamicTriangleMesh\DynamicTriangleMesh.450core.frag.spv DynamicTriangleMesh\DynamicTriangleMesh.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ShadowMapping\ShadowMapping.VShadow.450core.vert.spv ShadowMapping\ShadowMapping.VShadow.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ShadowMapping\ShadowMapping.VScene.450core.vert.spv ShadowMapping\ShadowMapping.VScene.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ShadowMapping\ShadowMapping.PScene.450core.frag.spv ShadowMapping\ShadowMapping.PScene.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o SemanticErrors\SemanticErrors.PSMain.450core.frag.spv SemanticErrors\SemanticErrors.PSMain.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceArrays\ResourceArrays.450core.vert.spv ResourceArrays\ResourceArrays.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceArrays\ResourceArrays.450core.frag.spv ResourceArrays\ResourceArrays.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceBinding\ResourceBinding.450core.comp.spv ResourceBinding\ResourceBinding.450core.comp
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceBinding\ResourceBinding.450core.vert.spv ResourceBinding\ResourceBinding.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ResourceBinding\ResourceBinding.450core.frag.spv ResourceBinding\ResourceBinding.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o ClearScreen\ClearScreen.450core.vert.spv ClearScreen\ClearScreen.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -o ClearScreen\ClearScreen.450core.frag.spv ClearScreen\ClearScreen.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -DXFB_OUTPUT=0 -o StreamOutput\StreamOutput.450core.vert.spv StreamOutput\StreamOutput.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -DXFB_OUTPUT=1 -o StreamOutput\StreamOutput.450core.vert.xfb.spv StreamOutput\StreamOutput.450core.vert
glslangValidator -V -DENABLE_SPIRV=1 -DXFB_OUTPUT=0 -o StreamOutput\StreamOutput.450core.tesc.spv StreamOutput\StreamOutput.450core.tesc
glslangValidator -V -DENABLE_SPIRV=1 -DXFB_OUTPUT=0 -o StreamOutput\StreamOutput.450core.tese.spv StreamOutput\StreamOutput.450core.tese
glslangValidator -V -DENABLE_SPIRV=1 -DXFB_OUTPUT=1 -o StreamOutput\StreamOutput.450core.tese.xfb.spv StreamOutput\StreamOutput.450core.tese
glslangValidator -V -DENABLE_SPIRV=1 -DXFB_OUTPUT=1 -o StreamOutput\StreamOutput.450core.geom.xfb.spv StreamOutput\StreamOutput.450core.geom
glslangValidator -V -DENABLE_SPIRV=1 -o StreamOutput\StreamOutput.450core.frag.spv StreamOutput\StreamOutput.450core.frag
glslangValidator -V -DENABLE_SPIRV=1 -o SamplerBuffer\SamplerBuffer.450core.comp.spv SamplerBuffer\SamplerBuffer.450core.comp
glslangValidator -V -DENABLE_SPIRV=1 -o ReadAfterWrite\ReadAfterWrite.450core.comp.spv ReadAfterWrite\ReadAfterWrite.450core.comp

echo DONE
pause
