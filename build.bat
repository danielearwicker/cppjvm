@echo off
if "empty%VS90COMNTOOLS%"=="empty" goto :NOVS
if "empty%VCINSTALLDIR%"=="empty" goto :VSVARS
goto :READY

:VSVARS
call "%VS90COMNTOOLS%vsvars32.bat"
goto :READY

:READY

make

goto :END

:NOVS
echo Could not find VS90COMNTOOLS variable - install Visual Studio 2008
:END
