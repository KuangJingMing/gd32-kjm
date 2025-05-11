chcp 65001
@echo off
setlocal enabledelayedexpansion

set SDK_PATH=..\SNR9912VR_SDK\projects\offline_asr_sample


set SDK_PATH=!SDK_PATH:/=\!
set TOOLS_PATH=!SDK_PATH!\..\..\tools


if exist *.bin (del *.bin)
!TOOLS_PATH!\ci-tool-kit.exe "cmd-info" "-V2"

if exist *.bin goto label_ok
echo cmd_info failed
!TOOLS_PATH!\tools\cmd_info_err.exe
goto end

:label_ok
echo cmd_info ok

:end
set TOOLS_PATH=!SDK_PATH!\tools

