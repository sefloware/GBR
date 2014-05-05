@echo off
echo ^>--------Simulation Beginning-------
setlocal EnableDelayedExpansion
for /f %%i in ('dir/b/ad-h^|findstr "[0-9]"') do (
call :RunOneKey %%i
if errorlevel 1 ( 
    echo ^>The process is aborted! & pause 
    exit /b !errorlevel!
	)
)
echo ^>----The process is completed successfully!---- & pause
exit /b 0

:RunOneKey
setlocal EnableDelayedExpansion
set cores=%cores%
set /p ts=<%1\times
if "%ts%"=="" echo ^>Error in getting times & exit /b 1
set /a index=ts-1
set state=0
for /l %%i in (%index%,-1,0) do (
set /a state+=1
if !state! equ %cores% set state=0
if %%i equ 0 set state=0
call :RunOneInstance %1 %%i !state!
if errorlevel 1 exit /b !errorlevel!
echo %%i > %1\times
)
echo 0 > %1\times 
exit /b 0

:RunOneInstance
set /p nskip=<mark
set skip=
if %nskip% NEQ 0 set skip="skip=%nskip%"
for /f %skip% %%i in (seeds) do (
set sds=%%i
goto StartEnd
)
exit /b 2
:StartEnd
set /a nskip+=1
echo %nskip% > mark

::make dir
set curpath=cd
cd %1 && (if exist %2 rd /s %2) && md %2 && cd %2
if errorlevel 1 echo ^>Error in making folder & cd %curpath% & exit /b 3

:: run
echo ^>build...
echo ^>Current seed: %sds%
echo ^>Current  dir: %cd%
if %3 equ 0 (
echo %sds% && echo ------------running--------------- && out.exe
if errorlevel 1 echo ^>Error in building and running exe & cd %curpath% & exit /b 4
) else (
echo %sds% && start /b out.exe
if errorlevel 1 echo ^>Error in building and running exe & cd %curpath% & exit /b 4
)
cd ..\..\
exit /b 0
