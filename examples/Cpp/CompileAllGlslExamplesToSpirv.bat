@echo off

echo ####### HelloOpenXR #######
call :CompileGlslToSpirv HelloOpenXR/Example.450core.vert
call :CompileGlslToSpirv HelloOpenXR/Example.450core.frag
call :CompileGlslToSpirv HelloOpenXR/Example.multiview.450core.vert
echo DONE

echo ####### HelloTriangle #######
call :CompileGlslToSpirv HelloTriangle/Example.450core.vert
call :CompileGlslToSpirv HelloTriangle/Example.450core.frag
echo DONE

echo ####### HelloTriangle #######
call :CompileGlslToSpirv HelloTriangle/Example.450core.vert
call :CompileGlslToSpirv HelloTriangle/Example.450core.frag
echo DONE

echo ####### MultiContext #######
call :CompileGlslToSpirv MultiContext/Example.450core.vert
call :CompileGlslToSpirv MultiContext/Example.450core.geom
call :CompileGlslToSpirv MultiContext/Example.450core.frag
echo DONE

echo ####### Tessellation #######
call :CompileGlslToSpirv Tessellation/Example.450core.vert
call :CompileGlslToSpirv Tessellation/Example.450core.tesc
call :CompileGlslToSpirv Tessellation/Example.450core.tese
call :CompileGlslToSpirv Tessellation/Example.450core.frag
echo DONE

exit /b 0

:CompileGlslToSpirv
call ../../scripts/CompileGlslToSpirv.bat %~1
exit /b 0

