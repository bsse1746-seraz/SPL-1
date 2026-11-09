@echo off
setlocal
pushd "%~dp0.." || exit /b 1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1"
if errorlevel 1 (
    popd
    pause
    exit /b 1
)
"build\timetablex.exe"
set "result=%errorlevel%"
popd
pause
exit /b %result%
