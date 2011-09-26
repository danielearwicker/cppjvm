@echo off
if "empty%VS100COMNTOOLS%"=="empty" goto :TRY2008
if "empty%VCINSTALLDIR%"=="empty" goto :VSVARS2010
goto :READY

:VSVARS2008
call "%VS90COMNTOOLS%vsvars32.bat"
goto :READY

:VSVARS2010
call "%VS100COMNTOOLS%vsvars32.bat"
goto :READY

:TRY2008
if "empty%VS90COMNTOOLS%"=="empty" goto :GIVEUP
if "empty%VCINSTALLDIR%"=="empty" goto :VSVARS2008
goto :READY

:READY
make
goto :END

:GIVEUP
echo Could not find VS90COMNTOOLS or VS100COMNTOOLS variables
echo - install Visual Studio 2008 or 2010
:END
