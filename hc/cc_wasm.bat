@echo off
setlocal
set /p flags=<%~dp0flags
if not defined CC set CC=clang
if not defined LD set LD=lld
%CC% -I%~dp0/src %flags% -target wasm32-unknown-unknown-unknown -fuse-ld="%LD%" %*
endlocal