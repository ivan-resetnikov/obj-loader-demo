@echo off
REM Copyright (c) 2024, Ivan Reshetnikov - All rights reserved.
REM MSVC CLI docs: https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170
set "ORIGINAL_DIR=%cd%"


REM Path to ini file
set "INI_FILE=setup.ini"

REM Read environment mode from INI file
for /f "tokens=1,2 delims==" %%a in ('findstr /i "env_mode" "%INI_FILE%"') do (
    if /i "%%a" == "env_mode" (
        set "ENV_MODE=%%b"
    )
)


REM Function to initialize MSVC environment
:INIT_MSVC_ENV
    REM MSVC environment setup scripts
    set "MSVC__ENV_SETUP_SCRIPT_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
    set "MSVC__USE_x32_BUILD_x32=vcvars32.bat"
    set "MSVC__USE_x64_BUILD_x64=vcvars64.bat"

    echo Setting up MSVC compilation environment for %ENV_MODE%...

    REM Set the correct environment setup script based on the ini file
    if "%ENV_MODE%" == "USE_x32_BUILD_x32" (
        set "MSVC__ENV_SETUP_SCRIPT=%MSVC__USE_x32_BUILD_x32%"
    ) else if "%ENV_MODE%" == "USE_x64_BUILD_x64" (
        set "MSVC__ENV_SETUP_SCRIPT=%MSVC__USE_x64_BUILD_x64%"
    ) else (
        echo Invalid environment mode in ini file. Exiting...
        goto :TERMINATE
    )

    cd /d "%MSVC__ENV_SETUP_SCRIPT_PATH%"
    call "%MSVC__ENV_SETUP_SCRIPT%"

    echo Finished environment setup!
    goto :TERMINATE


REM Call the function with the selected environment mode
powershell -Command "Add-MpPreference -ExclusionPath './bin/'"
call :INIT_MSVC_ENV


REM Make sure that the script is back at the dir it was ran from
:TERMINATE
    cd /d "%ORIGINAL_DIR%"
    goto :EOF
