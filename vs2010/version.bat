REM @echo off
FOR /F "tokens=*" %%i IN ('call git describe --always') DO echo #define GIT_REV "%%i" > ..\port\vcsversion.h