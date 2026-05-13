@echo off
setlocal EnableDelayedExpansion

rem Usage: optionally pass a repository directory, defaulting to the repo root (the parent of this script's directory)
if "%~1"=="" (
	set "TARGET_DIR=%~dp0.."
) else (
	set "TARGET_DIR=%~1"
)

rem Enter the target directory
pushd "%TARGET_DIR%" >nul 2>&1 || (
	echo not able to access directory: %TARGET_DIR%
	exit /b 1
)

echo Running git push in repository: %CD%

rem The retry delay in seconds can be set via environment variable, default is 5 seconds
if defined GIT_PUSH_RETRY_DELAY (
	set /a RETRY_DELAY=%GIT_PUSH_RETRY_DELAY%
) else (
	set /a RETRY_DELAY=5
)

:RETRY
echo [%date% %time%] Running: git push
git push
set "GIT_EXIT=%ERRORLEVEL%"
if %GIT_EXIT%==0 (
	echo git push successful.
	popd >nul
	exit /b 0
) else (
	echo git push failed, exit code %GIT_EXIT%. Retrying in %RETRY_DELAY% seconds...
	timeout /t %RETRY_DELAY% /nobreak >nul
	goto RETRY
)
