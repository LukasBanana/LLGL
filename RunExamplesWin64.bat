@echo off

set BUILD_DIR="build_win64/build"

:Start
echo select example:
echo   1.) Hello Triangle
echo   2.) Tessellation
echo   3.) Texturing
echo   4.) Queries
echo   5.) Render Target
echo   6.) Multi Context
echo   7.) Buffer Array
echo   8.) Compute Shader
echo   9.) Stream Output
echo  10.) Instancing
echo  11.) Post Processing
echo  12.) Shadow Mapping
echo  13.) Multi Renderer
echo  14.) Stencil Buffer
echo  15.) Volume Rendering
echo  16.) Cloth Physics
echo  17.) Mapping
echo  18.) Animation

:Prompt
set /p "Input="

if not defined Input goto Prompt

if "%Input%" EQU "1" (
    cd %~dp0\examples\Cpp\HelloTriangle
    %~dp0\%BUILD_DIR%\Debug\Example_HelloTriangleD.exe
)

if "%Input%" EQU "2" (
    cd %~dp0\examples\Cpp\Tessellation
    %~dp0\%BUILD_DIR%\Debug\Example_TessellationD.exe
)

if "%Input%" EQU "3" (
    cd %~dp0\examples\Cpp\Texturing
    %~dp0\%BUILD_DIR%\Debug\Example_TexturingD.exe
)

if "%Input%" EQU "4" (
    cd %~dp0\examples\Cpp\Queries
    %~dp0\%BUILD_DIR%\Debug\Example_QueriesD.exe
)

if "%Input%" EQU "5" (
    cd %~dp0\examples\Cpp\RenderTarget
    %~dp0\%BUILD_DIR%\Debug\Example_RenderTargetD.exe
)

if "%Input%" EQU "6" (
    cd %~dp0\examples\Cpp\MultiContext
    %~dp0\%BUILD_DIR%\Debug\Example_MultiContextD.exe
)

if "%Input%" EQU "7" (
    cd %~dp0\examples\Cpp\BufferArray
    %~dp0\%BUILD_DIR%\Debug\Example_BufferArrayD.exe
)

if "%Input%" EQU "8" (
    cd %~dp0\examples\Cpp\ComputeShader
    %~dp0\%BUILD_DIR%\Debug\Example_ComputeShaderD.exe
)

if "%Input%" EQU "9" (
    cd %~dp0\examples\Cpp\StreamOutput
    %~dp0\%BUILD_DIR%\Debug\Example_StreamOutputD.exe
)

if "%Input%" EQU "10" (
    cd %~dp0\examples\Cpp\Instancing
    %~dp0\%BUILD_DIR%\Debug\Example_InstancingD.exe
)

if "%Input%" EQU "11" (
    cd %~dp0\examples\Cpp\PostProcessing
    %~dp0\%BUILD_DIR%\Debug\Example_PostProcessingD.exe
)

if "%Input%" EQU "12" (
    cd %~dp0\examples\Cpp\ShadowMapping
    %~dp0\%BUILD_DIR%\Debug\Example_ShadowMappingD.exe
)

if "%Input%" EQU "13" (
    cd %~dp0\examples\Cpp\MultiRenderer
    %~dp0\%BUILD_DIR%\Debug\Example_MultiRendererD.exe
)

if "%Input%" EQU "14" (
    cd %~dp0\examples\Cpp\StencilBuffer
    %~dp0\%BUILD_DIR%\Debug\Example_StencilBufferD.exe
)

if "%Input%" EQU "15" (
    cd %~dp0\examples\Cpp\VolumeRendering
    %~dp0\%BUILD_DIR%\Debug\Example_VolumeRenderingD.exe
)

if "%Input%" EQU "16" (
    cd %~dp0\examples\Cpp\ClothPhysics
    %~dp0\%BUILD_DIR%\Debug\Example_ClothPhysicsD.exe
)

if "%Input%" EQU "17" (
    cd %~dp0\examples\Cpp\Mapping
    %~dp0\%BUILD_DIR%\Debug\Example_MappingD.exe
)

if "%Input%" EQU "18" (
    cd %~dp0\examples\Cpp\Animation
    %~dp0\%BUILD_DIR%\Debug\Example_AnimationD.exe
)

goto Start
