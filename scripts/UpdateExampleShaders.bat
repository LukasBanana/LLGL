@echo off

pushd "%~dp0.."
python scripts\TranslateShaders.py -c
set RESULT=%ERRORLEVEL%
popd

if %RESULT% neq 0 ( pause )

exit /B %RESULT%

