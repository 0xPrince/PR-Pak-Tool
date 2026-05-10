::PR Pak Tool Unpacker Helper Script
::Supports Multiple Pak File Processing 


@echo off
setlocal EnableDelayedExpansion

::User Options
::true if you want additional logs(pak entries name, info.. etc) in log file 
set bShowLogs=false

::vars
set AppExeName=PRPakTool
set AppPath="%~dp0%AppExeName%.exe"
set AppExtraArgs=

:EntryPoint
echo PR Pak Tool Unpacker Helper Script
echo.
 
IF NOT EXIST %AppPath% (
echo ERROR: %AppExeName% Not Found in The Current Folder
goto :ExitPoint
)
IF [%1]==[] goto :ShowUsage

IF %bShowLogs%==true set AppExtraArgs= %AppExtraArgs% --verbose
goto :ProcessFiles

:ShowUsage
echo Usage:
echo 	%~nx0 "<PATH_TO_PAK_FILE>"
echo 	OR Drag and Drop One or More Pak File(s) Directly onto this script(%~nx0)
goto :ExitPoint

:ExitPoint
echo Script Exited
pause
exit

:ProcessFiles
IF [%1]==[] goto :ExitPoint

set PakPath=%1
echo Unpacking %PakPath%
START /B /WAIT %AppPath% %AppPath% %PakPath% %AppExtraArgs% > %PakPath%_unpack.log 
IF %ERRORLEVEL%==0 (
echo Unpacking Process Succeed
) else (
echo Unpacking Failed, Program Returned Code: %ERRORLEVEL%
)
SHIFT
goto :ProcessFiles
