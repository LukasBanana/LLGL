@echo OFF

set SOURCE_DIR=%~dp0
set OUTPUT_DIR=build

REM Ensure we are inside the repository folder
if not exist "CMakeLists.txt" (
    echo error: file not found: CMakeLists.txt
    exit 1
)

REM Make output build folder
if not "%1" == "" (
    if "%2" == "" (
        set OUTPUT_DIR="%1"
    ) else (
        echo error: too many arguemnts
        echo usage: BuildWin64.bat [OUTPUT_DIR]
        exit 1
    )
)

IF NOT EXIST %OUTPUT_DIR% (
    mkdir %OUTPUT_DIR%
)

cd %OUTPUT_DIR%

REM Checkout external depenencies
set GAUSSIAN_LIB_DIR="%OUTPUT_DIR%\GaussianLib\include"

if not exist %GAUSSIAN_LIB_DIR% (
    REM Find executable path of <git>
    REM ... TODO ...
    
    REM Clone third party into build folder
    git clone https://github.com/LukasBanana/GaussianLib.git
)

REM Build into output directory
cmake -DLLGL_BUILD_RENDERER_OPENGL=ON -DLLGL_BUILD_RENDERER_DIRECT3D11=ON -DLLGL_BUILD_RENDERER_DIRECT3D12=ON -DLLGL_BUILD_EXAMPLES=ON -DGaussLib_INCLUDE_DIR:STRING=%GAUSSIAN_LIB_DIR% -DCMAKE_BUILD_TYPE=Release -S %SOURCE_DIR%
cmake --build .
