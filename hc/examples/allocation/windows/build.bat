@echo off
setlocal
set "root_dir=%~dp0..\..\.."

set "FLAGS=-l:kernel32.lib"
call "%root_dir%\tools\build\exe.bat" "%~dp0" allocation

endlocal
