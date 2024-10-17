@echo off

echo ####### Animation #######
call :CompileGlslToSpirv Animation/Example.450core.vert
call :CompileGlslToSpirv Animation/Example.450core.frag
echo DONE

echo ####### ClothPhysics #######
call :CompileGlslToSpirv ClothPhysics/Example.VS.450core.vert
call :CompileGlslToSpirv ClothPhysics/Example.PS.450core.frag
call :CompileGlslToSpirv ClothPhysics/Example.CSForces.450core.comp
call :CompileGlslToSpirv ClothPhysics/Example.CSStretchConstraints.450core.comp
call :CompileGlslToSpirv ClothPhysics/Example.CSRelaxation.450core.comp
echo DONE

echo ####### Fonts #######
call :CompileGlslToSpirv Fonts/Example.450core.vert
call :CompileGlslToSpirv Fonts/Example.450core.frag
echo DONE

echo ####### HelloGame #######
call :CompileGlslToSpirv HelloGame/HelloGame.VSInstance.450core.vert
call :CompileGlslToSpirv HelloGame/HelloGame.PSInstance.450core.frag
call :CompileGlslToSpirv HelloGame/HelloGame.VSGround.450core.vert
call :CompileGlslToSpirv HelloGame/HelloGame.PSGround.450core.frag
echo DONE

echo ####### HelloTriangle #######
call :CompileGlslToSpirv HelloTriangle/Example.450core.vert
call :CompileGlslToSpirv HelloTriangle/Example.450core.frag
echo DONE

echo ####### HelloTriangle #######
call :CompileGlslToSpirv HelloTriangle/Example.450core.vert
call :CompileGlslToSpirv HelloTriangle/Example.450core.frag
echo DONE

echo ####### IndirectDraw #######
call :CompileGlslToSpirv IndirectDraw/Example.comp
call :CompileGlslToSpirv IndirectDraw/Example.vert
call :CompileGlslToSpirv IndirectDraw/Example.frag
echo DONE

echo ####### Instancing #######
call :CompileGlslToSpirv Instancing/Example.450core.vert
call :CompileGlslToSpirv Instancing/Example.450core.frag
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

echo ####### PBR #######
call :CompileGlslToSpirv PBR/Example.Mesh.450core.vert
call :CompileGlslToSpirv PBR/Example.Mesh.450core.frag
call :CompileGlslToSpirv PBR/Example.Sky.450core.vert
call :CompileGlslToSpirv PBR/Example.Sky.450core.frag
echo DONE

echo ####### PostProcessing #######
call :CompileGlslToSpirv PostProcessing/Scene.450core.vert
call :CompileGlslToSpirv PostProcessing/Scene.450core.frag
call :CompileGlslToSpirv PostProcessing/PostProcess.450core.vert
call :CompileGlslToSpirv PostProcessing/Final.450core.frag
call :CompileGlslToSpirv PostProcessing/Blur.450core.frag
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

echo ####### StencilBuffer #######
call :CompileGlslToSpirv StencilBuffer/Stencil.450core.vert
call :CompileGlslToSpirv StencilBuffer/Scene.450core.vert
call :CompileGlslToSpirv StencilBuffer/Scene.450core.frag
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

echo ####### VolumeRendering #######
call :CompileGlslToSpirv VolumeRendering/Example.450core.vert
call :CompileGlslToSpirv VolumeRendering/Example.450core.frag
echo DONE

exit /b 0

:CompileGlslToSpirv
call ../../scripts/CompileGlslToSpirv.bat %~1
exit /b 0

