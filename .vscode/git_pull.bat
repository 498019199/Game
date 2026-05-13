@echo off
setlocal EnableDelayedExpansion

rem 使用方法: 可传入可选参数 指定仓库目录, 默认为仓库根目录（本脚本所在目录的上一级）
if "%~1"=="" (
	set "TARGET_DIR=%~dp0.."
) else (
	set "TARGET_DIR=%~1"
)

rem 进入目标目录
pushd "%TARGET_DIR%" >nul 2>&1 || (
	echo not able to access directory: %TARGET_DIR%
	exit /b 1
)

echo 正在对仓库执行 git pull (目录: %CD%)

rem 可通过环境变量设置重试间隔（秒），默认 5 秒
if defined GIT_PULL_RETRY_DELAY (
	set /a RETRY_DELAY=%GIT_PULL_RETRY_DELAY%
) else (
	set /a RETRY_DELAY=5
)

:RETRY
echo [%date% %time%] 运行: git pull
git pull
set "GIT_EXIT=%ERRORLEVEL%"
if %GIT_EXIT%==0 (
	echo git pull successful.
	popd >nul
	exit /b 0
) else (
	echo git pull failed, exit code %GIT_EXIT%. Retrying in %RETRY_DELAY% seconds...
	timeout /t %RETRY_DELAY% /nobreak >nul
	goto RETRY
)
