@echo off

if not exist WrapperGen\__main__.py (
    echo Error: Missing WrapperGen\__main__.py script in current folder
    exit 1
)

set INCLUDE=..\include\LLGL
set CINCLUDE=..\include\LLGL-C

set INPUT=^
    %INCLUDE%\BufferFlags.h ^
    %INCLUDE%\CanvasFlags.h ^
    %INCLUDE%\CommandBufferFlags.h ^
    %INCLUDE%\DisplayFlags.h ^
    %INCLUDE%\Format.h ^
    %INCLUDE%\FragmentAttribute.h ^
    %INCLUDE%\ImageFlags.h ^
    %INCLUDE%\IndirectArguments.h ^
    %INCLUDE%\Log.h ^
    %INCLUDE%\Key.h ^
    %INCLUDE%\PipelineLayoutFlags.h ^
    %INCLUDE%\PipelineStateFlags.h ^
    %INCLUDE%\QueryHeapFlags.h ^
    %INCLUDE%\RenderingDebuggerFlags.h ^
    %INCLUDE%\RenderPassFlags.h ^
    %INCLUDE%\RenderSystemFlags.h ^
    %INCLUDE%\RenderTargetFlags.h ^
    %INCLUDE%\ResourceFlags.h ^
    %INCLUDE%\ResourceHeapFlags.h ^
    %INCLUDE%\SamplerFlags.h ^
    %INCLUDE%\ShaderFlags.h ^
    %INCLUDE%\ShaderReflection.h ^
    %INCLUDE%\SwapChainFlags.h ^
    %INCLUDE%\SystemValue.h ^
    %INCLUDE%\TextureFlags.h ^
    %INCLUDE%\Types.h ^
    %INCLUDE%\VertexAttribute.h ^
    %INCLUDE%\WindowFlags.h

set INPUT_FN=^
    %INPUT% ^
    %CINCLUDE%\Buffer.h ^
    %CINCLUDE%\BufferArray.h ^
    %CINCLUDE%\Canvas.h ^
    %CINCLUDE%\CommandBuffer.h ^
    %CINCLUDE%\CommandQueue.h ^
    %CINCLUDE%\Display.h ^
    %CINCLUDE%\Log.h ^
    %CINCLUDE%\PipelineCache.h ^
    %CINCLUDE%\PipelineLayout.h ^
    %CINCLUDE%\PipelineState.h ^
    %CINCLUDE%\QueryHeap.h ^
    %CINCLUDE%\RenderingDebugger.h ^
    %CINCLUDE%\RenderSystem.h ^
    %CINCLUDE%\RenderSystemChild.h ^
    %CINCLUDE%\RenderTarget.h ^
    %CINCLUDE%\Report.h ^
    %CINCLUDE%\Resource.h ^
    %CINCLUDE%\ResourceHeap.h ^
    %CINCLUDE%\Shader.h ^
    %CINCLUDE%\Surface.h ^
    %CINCLUDE%\SwapChain.h ^
    %CINCLUDE%\Texture.h ^
    %CINCLUDE%\Timer.h ^
    %CINCLUDE%\TypeNames.h ^
    %CINCLUDE%\Window.h

REM Generate wrapper for C99, C#, Go
call :Generate .\LLGLWrapper.h -c99
call :Generate .\LLGLWrapper.cs -csharp -fn
call :Generate .\LLGLWrapper.go -golang

exit /B 0

:Generate
set OUTPUT=%~1
set LANGUAGE=%~2
set FUNCTIONS=%~3

REM Generate wrapper
if "%FUNCTIONS%"=="" (
    set ARGS=WrapperGen "-name=LLGLWrapper" %LANGUAGE% %INPUT%
) else (
    set ARGS=WrapperGen "-name=LLGLWrapper" %LANGUAGE% %FUNCTIONS% %INPUT_FN%
)

REM Run Python script to parse LLGL headers and write result into output file
python %ARGS% > %OUTPUT%

REM Repeat command if an error occurred
if %ERRORLEVEL% NEQ 0 (
    python %ARGS%
)

exit /B %ERRORLEVEL%

