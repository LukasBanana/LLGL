@echo off

if not exist .\ParseLLGLHeader.py (
    echo Error: Missing ParseLLGLHeader.py script in current folder
    exit 1
)

set INCLUDE=..\include\LLGL

set INPUT=^
    %INCLUDE%\BufferFlags.h ^
    %INCLUDE%\CanvasFlags.h ^
    %INCLUDE%\CommandBufferFlags.h ^
    %INCLUDE%\DisplayFlags.h ^
    %INCLUDE%\Format.h ^
    %INCLUDE%\FragmentAttribute.h ^
    %INCLUDE%\ImageFlags.h ^
    %INCLUDE%\IndirectArguments.h ^
    %INCLUDE%\Key.h ^
    %INCLUDE%\PipelineLayoutFlags.h ^
    %INCLUDE%\PipelineStateFlags.h ^
    %INCLUDE%\QueryHeapFlags.h ^
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

REM Generate wrapper for C99, C#
call :Generate c99 .\LLGLWrapper.h
REM call :Generate csharp .\LLGLWrapper.cs

exit /B 0

:Generate
set LANGUAGE=%~1
set OUTPUT=%~2

REM Generate wrapper
set ARGS=.\ParseLLGLHeader.py -%LANGUAGE% "-name=LLGLWrapper" %INPUT%

REM Run Python script to parse LLGL headers and write result into output file
python3 %ARGS% > %OUTPUT%

REM Repeat command if an error occurred
if %ERRORLEVEL% NEQ 0 (
    python3 %ARGS%
)

exit /B %ERRORLEVEL%

