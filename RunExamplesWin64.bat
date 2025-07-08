@echo off
setlocal EnableDelayedExpansion

REM Get build directory
set BUILD_DIR=build_win64\build

if not "%1" == "" (
    set BUILD_DIR=%1
    if "!BUILD_DIR:~-1!"=="\" set BUILD_DIR=!BUILD_DIR:~0,-1!
    if "!BUILD_DIR:~-1!"=="/" set BUILD_DIR=!BUILD_DIR:~0,-1!
    if not exist "!BUILD_DIR!\Debug\LLGLD.dll" (
        if not exist "!BUILD_DIR!\Release\LLGL.dll" (
            set BUILD_DIR=!BUILD_DIR!\build
        )
    )
)

REM Find all example projects
set EXAMPLES=""
for /F "TOKENS=* USEBACKQ" %%F in (`dir /B /AD examples\Cpp`) do (
    if !EXAMPLES! EQU "" (
        set EXAMPLES=%%F
    ) else (
        set EXAMPLES=!EXAMPLES! %%F
    )
)

:Start
echo Select example:

echo  q.^)^ QUIT
echo  --------

set /A NUM_EXAMPLES = 0
for %%F in (!EXAMPLES!) do (
    set /A NUM_EXAMPLES += 1
    set EXAMPLE_NAME[!NUM_EXAMPLES!]=%%F
    if !NUM_EXAMPLES! LSS 10 (
        echo  !NUM_EXAMPLES!.^)^ %%F
    ) else (
        echo !NUM_EXAMPLES!.^)^ %%F
    )
)

:Prompt
set /P "INPUT="

if not defined INPUT goto Prompt

if "%INPUT%"=="q" goto End
if "%INPUT%"=="Q" goto End

if %INPUT% LEQ %NUM_EXAMPLES% (
    set SELECTION=!EXAMPLE_NAME[%INPUT%]!
    pushd %~dp0\examples\Cpp\!SELECTION!
    if exist "%~dp0\%BUILD_DIR%\Debug\Example_!SELECTION!D.exe" (
        %~dp0\%BUILD_DIR%\Debug\Example_!SELECTION!D.exe
    ) else if exist "%~dp0\%BUILD_DIR%\RelWithDebInfo\Example_!SELECTION!.exe" (
        %~dp0\%BUILD_DIR%\RelWithDebInfo\Example_!SELECTION!.exe
    ) else (
        %~dp0\%BUILD_DIR%\Release\Example_!SELECTION!.exe
    )
    popd
)

goto Start

:End
