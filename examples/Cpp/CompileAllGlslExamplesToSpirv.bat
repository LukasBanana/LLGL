@echo off

echo ####### HelloTriangle #######
call :CompileGlslToSpirv HelloTriangle/Example.450core.vert
call :CompileGlslToSpirv HelloTriangle/Example.450core.frag
echo DONE

echo ####### Tessellation #######
call :CompileGlslToSpirv Tessellation/Example.450core.vert
call :CompileGlslToSpirv Tessellation/Example.450core.tesc
call :CompileGlslToSpirv Tessellation/Example.450core.tese
call :CompileGlslToSpirv Tessellation/Example.450core.frag
echo DONE

echo ####### Texturing #######
call :CompileGlslToSpirv Texturing/Example.450core.vert
call :CompileGlslToSpirv Texturing/Example.450core.frag
echo DONE

echo ####### Queries #######
call :CompileGlslToSpirv Queries/Example.450core.vert
call :CompileGlslToSpirv Queries/Example.450core.frag
echo DONE

echo ####### RenderTarget #######
call :CompileGlslToSpirv RenderTarget/Example.450core.vert
call :CompileGlslToSpirv RenderTarget/Example.450core.frag
echo DONE

echo ####### ResourceBinding #######
call :CompileGlslToSpirv ResourceBinding/Example.450core.vert
call :CompileGlslToSpirv ResourceBinding/Example.450core.frag
echo DONE

echo ####### ShadowMapping #######
call :CompileGlslToSpirv ShadowMapping/ShadowMap.450core.vert
call :CompileGlslToSpirv ShadowMapping/Scene.450core.vert
call :CompileGlslToSpirv ShadowMapping/Scene.450core.frag
echo DONE

echo ####### MultiContext #######
call :CompileGlslToSpirv MultiContext/Example.450core.vert
call :CompileGlslToSpirv MultiContext/Example.450core.geom
call :CompileGlslToSpirv MultiContext/Example.450core.frag
echo DONE

echo ####### MultiRenderer #######
call :CompileGlslToSpirv MultiRenderer/Example.450core.vert
call :CompileGlslToSpirv MultiRenderer/Example.450core.frag
echo DONE

echo ####### MultiThreading #######
call :CompileGlslToSpirv MultiThreading/Example.450core.vert
call :CompileGlslToSpirv MultiThreading/Example.450core.frag
echo DONE

echo ####### BufferArray #######
call :CompileGlslToSpirv BufferArray/Example.450core.vert
call :CompileGlslToSpirv BufferArray/Example.450core.frag
echo DONE

echo ####### ComputeShader #######
call :CompileGlslToSpirv ComputeShader/Example.comp
call :CompileGlslToSpirv ComputeShader/Example.vert
call :CompileGlslToSpirv ComputeShader/Example.frag
echo DONE

echo ####### Instancing #######
call :CompileGlslToSpirv Instancing/Example.450core.vert
call :CompileGlslToSpirv Instancing/Example.450core.frag
echo DONE

echo ####### PostProcessing #######
call :CompileGlslToSpirv PostProcessing/Scene.450core.vert
call :CompileGlslToSpirv PostProcessing/Scene.450core.frag
call :CompileGlslToSpirv PostProcessing/PostProcess.450core.vert
call :CompileGlslToSpirv PostProcessing/Final.450core.frag
call :CompileGlslToSpirv PostProcessing/Blur.450core.frag
echo DONE

echo ####### StencilBuffer #######
call :CompileGlslToSpirv StencilBuffer/Stencil.450core.vert
call :CompileGlslToSpirv StencilBuffer/Scene.450core.vert
call :CompileGlslToSpirv StencilBuffer/Scene.450core.frag
echo DONE

echo ####### StreamOutput #######
call :CompileGlslToSpirv StreamOutput/Example.450core.vert
call :CompileGlslToSpirv StreamOutput/Example.450core.geom
call :CompileGlslToSpirv StreamOutput/Example.450core.frag
echo DONE

echo ####### Animation #######
call :CompileGlslToSpirv Animation/Example.450core.vert
call :CompileGlslToSpirv Animation/Example.450core.frag
echo DONE

echo ####### VolumeRendering #######
call :CompileGlslToSpirv VolumeRendering/Example.450core.vert
call :CompileGlslToSpirv VolumeRendering/Example.450core.frag
echo DONE

echo ####### ClothPhysics #######
call :CompileGlslToSpirv ClothPhysics/Example.VS.450core.vert
call :CompileGlslToSpirv ClothPhysics/Example.PS.450core.frag
call :CompileGlslToSpirv ClothPhysics/Example.CSForces.450core.comp
call :CompileGlslToSpirv ClothPhysics/Example.CSStretchConstraints.450core.comp
call :CompileGlslToSpirv ClothPhysics/Example.CSRelaxation.450core.comp
echo DONE

echo ####### UnorderedAccess #######
call :CompileGlslToSpirv UnorderedAccess/Example.450core.vert
call :CompileGlslToSpirv UnorderedAccess/Example.450core.frag
call :CompileGlslToSpirv UnorderedAccess/Example.450core.comp
echo DONE

echo ####### Mapping #######
call :CompileGlslToSpirv Mapping/Example.450core.vert
call :CompileGlslToSpirv Mapping/Example.450core.frag
echo DONE

exit /b 0

:CompileGlslToSpirv
call ../../scripts/CompileGlslToSpirv.bat %~1
exit /b 0

