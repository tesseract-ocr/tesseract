@echo off
FOR /F "tokens=*" %%i IN ('call git describe --tags --always') DO echo #define GIT_REV "%%i" > ..\port\vcsversion.h