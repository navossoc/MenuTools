@echo off
cd /d "%~dp0"
for %%i in (.) do set fn=%%~nxi

attrib . | find "R"
if errorlevel 1 goto:r+
if errorlevel 0 goto:r-

:r+
echo +r
if exist desktop.ini goto:r++
echo [.ShellClassInfo] > desktop.ini
echo ConfirmFileOp=0 >> desktop.ini
echo IconFile=folder.ico >> desktop.ini
echo IconIndex=0 >> desktop.ini
echo InfoTip="%fn%" >> desktop.ini
:r++
attrib +r .
attrib +h desktop.ini
attrib +h %~nx0
attrib +h messages.*
attrib +h folder.*
goto:refresh

:r-
echo -r
attrib -r .
attrib -h -s desktop.ini
attrib -h %~nx0
attrib -h messages.*
attrib -h folder.*
goto:refresh

:refresh
call ie4uinit -show
@REM pause

