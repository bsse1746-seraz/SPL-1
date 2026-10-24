@echo off
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 (
 echo C++ compiler g++ was not found. See START_HERE.txt for IDE instructions.
 pause
 exit /b 1
)
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o timetablex.exe
if errorlevel 1 (
 pause
 exit /b 1
)
timetablex.exe
pause
