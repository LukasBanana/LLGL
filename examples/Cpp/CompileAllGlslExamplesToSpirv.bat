@echo off

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

