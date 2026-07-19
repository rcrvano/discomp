@echo off
if "%1"=="-l" GOTO log

java -cp bin GServ
goto :end

:log
set VDATE=%date:~3%
set VTIME=%time:~0,-3%
set VTIME=%VTIME::=.%
set VTIME=%VTIME: =0%

for /f "tokens=2,3,4 delims=. " %%i in ("%date%") do (
set LOGFILE=logx\%%k.%%j.%%i_%VTIME%.log
)
java -cp bin GServ 1>%LOGFILE% 2>&1

:end