@echo off
setlocal disabledelayedexpansion
set "script_dir=%~dp0"
set "script_dir=%script_dir:~0,-1%"
set "root_dir=%script_dir%\..\.."
goto start

:build
setlocal
set "out_dir=%root_dir%\..\hc-out\%out_path%\%ARCH%"
if not exist "%out_dir%" (
    mkdir "%out_dir%"
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
)

set "FLAGS=-L^"%out_dir%^" %FLAGS% %~1"
if defined ASSEMBLY (
    call "%root_dir%\cc_elf.bat" %debug_flags% -S -o "%out_dir%\debug-%source_name%%ext%.s" "%source%" %FLAGS%
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
    call "%root_dir%\cc_elf.bat" %release_flags% -S -o "%out_dir%\%source_name%%ext%.s" "%source%" %FLAGS%
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
)
call "%root_dir%\cc_elf.bat" %debug_flags% -o "%out_dir%\debug-%source_name%%ext%" "%source%" %FLAGS%
if not errorlevel 0 exit /b
if errorlevel 1 exit /b
call "%root_dir%\cc_elf.bat" %release_flags% -o "%out_dir%\%source_name%%ext%" "%source%" %FLAGS%
if not errorlevel 0 exit /b
if errorlevel 1 exit /b
if defined STRIP_OPT (
    "%llvm_prefix%llvm-objcopy" %STRIP_OPT% "%out_dir%\%source_name%%ext%"
)
if not errorlevel 0 exit /b
if errorlevel 1 exit /b

if not defined NO_ANALYSIS (
    call "%root_dir%\cc_elf.bat" %debug_flags% %analyse_flags% "%source%" %FLAGS%
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
    call "%root_dir%\cc_elf.bat" %release_flags% %analyse_flags% "%source%" %FLAGS%
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
)
exit /b

:start
if defined LLVM set "llvm_prefix=%LLVM%\bin\"

set "source=%~1"
set "source_name=%~n1"
set "out_path=%~2"
set "ext=%~3"

set "analyse_flags=--analyze --analyzer-output text -Xclang -analyzer-opt-analyze-headers"
set "debug_flags=-fsanitize-undefined-trap-on-error -fsanitize=undefined -g -Dhc_DEBUG"
set "release_flags=-fomit-frame-pointer -s -Os"

if not defined NO_X86_64 (
    set "ARCH=x86_64" & call :build "%FLAGS_X86_64%"
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
)
if not defined NO_AARCH64 (
    set "ARCH=aarch64" & call :build "%FLAGS_AARCH64%"
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
)
if not defined NO_RISCV64 (
    set "ARCH=riscv64" & call :build "%FLAGS_RISCV64%"
    if not errorlevel 0 exit /b
    if errorlevel 1 exit /b
)
