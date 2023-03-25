@echo off

if "%1"=="" (
    echo Usage: CompileGlslToSpirv.bat FILE [STAGE]
    exit 1
)

set INPUT_FILE=%1
set OUTPUT_FILE=%INPUT_FILE%.spv

if "%2"=="" (
    glslangValidator -V -o %OUTPUT_FILE% %INPUT_FILE%
) else (
    glslangValidator -V -S %2 -o %OUTPUT_FILE% %INPUT_FILE%
)
