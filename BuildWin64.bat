@echo OFF

set SOURCE_DIR=%~dp0
set OUTPUT_DIR=build_win64
set BUILD_TYPE=Debug
set BUILD_ARCH=x64
set ENABLE_EXAMPLES=ON
set ENABLE_TESTS=ON
set STATIC_LIB=OFF
set VERBOSE=0
set PROJECT_ONLY=0

REM Ensure we are inside the repository folder
if not exist "CMakeLists.txt" (
    echo error: file not found: CMakeLists.txt
    exit 1
)

REM Make output build folder
if not "%1" == "" (
    if "%2" == "" (
        set OUTPUT_DIR=%1
    ) else (
        echo error: too many arguemnts
        echo usage: BuildWin64.bat [OUTPUT_DIR]
        exit 1
    )
)

if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

REM Checkout external depenencies
set GAUSSIAN_LIB_DIR=GaussianLib\include

if exist "external\%GAUSSIAN_LIB_DIR%" (
    set GAUSSIAN_LIB_DIR=external\%GAUSSIAN_LIB_DIR%
) else (
    if not exist "%GAUSSIAN_LIB_DIR%" (
        REM Clone third party into build folder
        pushd %OUTPUT_DIR%
        git clone https://github.com/LukasBanana/GaussianLib.git
        popd
    )
    set GAUSSIAN_LIB_DIR=%OUTPUT_DIR%\%GAUSSIAN_LIB_DIR%
)

REM Print additional information if in verbose mode
if %VERBOSE% == 1 (
    echo GAUSSIAN_LIB_DIR=%GAUSSIAN_LIB_DIR%
)

REM Build into output directory
set OPTIONS= ^
    -DLLGL_BUILD_WRAPPER_C99=ON ^
    -DLLGL_BUILD_RENDERER_OPENGL=ON ^
    -DLLGL_BUILD_RENDERER_DIRECT3D11=ON ^
    -DLLGL_BUILD_RENDERER_DIRECT3D12=ON ^
    -DLLGL_BUILD_EXAMPLES=%ENABLE_EXAMPLES% ^
    -DLLGL_BUILD_TESTS=%ENABLE_TESTS% ^
    -DLLGL_BUILD_STATIC_LIB=%STATIC_LIB% ^
    -DGaussLib_INCLUDE_DIR:STRING="%GAUSSIAN_LIB_DIR%" ^
    -A %BUILD_ARCH% ^
    -B "%OUTPUT_DIR%" ^
    -S "%SOURCE_DIR%"

if %PROJECT_ONLY% == 0 (
    cmake %OPTIONS%
    cmake --build "%OUTPUT_DIR%" --config %BUILD_TYPE%
) else (
    cmake %OPTIONS%
)
