echo off

rem Visual Studio

del /Q .vs
rd /S /Q .vs
del *.sln

rem Visual Studio Code

del /Q .vscode
rd /S /Q .vscode
del *.code-workspace

rem Unreal Engine

del /Q Binaries
rd /S /Q Binaries
rem Build - build settings
del /Q "Build\WindowsNoEditor"
rd /S /Q "Build\WindowsNoEditor"
del /Q "Build\WindowsServer"
rd /S /Q "Build\WindowsServer"
del /Q "Build\LinuxServer"
rd /S /Q "Build\LinuxServer"
rem Intermediate - temporary files generated when you compile your code
del /Q Intermediate
rd /S /Q Intermediate
rem Saved - local log & configuration files, screenshots, auto-saves etc.
del /Q Saved
rd /S /Q Saved