
@echo off

if "%2" == "" (
    echo "usage: ReadFileAsHexStringDecl.bat INPUT DECL"
    exit 1
)

set INPUT=%1
set DECL=%2

set SCRIPT_PATH=%~dp0ReadFileAsHexString.py
echo static const char* %DECL% =
python3 "%SCRIPT_PATH%" -spaces 4 -offsets cxx "%INPUT%"
echo ;
echo|set /p="static const std::size_t %DECL%_Len = "
for /F "tokens=*" %%F in ('python3 "%SCRIPT_PATH%" -len -paren "%INPUT%"') do set STR_LEN_LINE=%%F
echo %STR_LEN_LINE%;
