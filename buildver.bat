@echo off
if "empty%VSVER%"=="empty" goto :NOVS
if "empty%VCINSTALLDIR%"=="empty" goto :VSVARS
goto :READY

:VSVARS
call "%VSVER%vsvars32.bat"
goto :READY

:READY

make -rR

goto :END

:NOVS
echo Could not find Visual Studio
:END
