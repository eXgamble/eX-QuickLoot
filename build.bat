@echo off
rem Configure + build the QuickLoot fork from a VS 2026 developer environment (Ninja, VS's own vcpkg).
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 >nul || exit /b 1
cd /d "%~dp0"
rem The skse-dev-rules checker, when installed (a Claude Code skill): warnings print, an error stops the build.
set "SKSE_LINT=%USERPROFILE%\.claude\skills\skse-dev-rules\scripts\skse_lint.py"
if exist "%SKSE_LINT%" python "%SKSE_LINT%" src
cmake --preset release || exit /b 2
cmake --build --preset release || exit /b 3
echo BUILD OK
