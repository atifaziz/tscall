@echo off
setlocal EnableDelayedExpansion
if not defined VSCMD_VER (
    set VSVARS=
    for %%e in (Community Professional Enterprise) do (
        if not defined VSVARS (
            set VSVARS_TMP="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\%%e\VC\Auxiliary\Build\vcvarsall.bat"
            if exist !VSVARS_TMP! (
                call !VSVARS_TMP! x64
                if not !ERRORLEVEL!==0 exit /b !ERRORLEVEL!
                set VSVARS=%VSVARS_TMP%
            )
        )
    )
)
if not defined VSCMD_VER goto :vs_not_installed
pushd "%~dp0"
if "%1"=="" set CLARGS=/MD /O1
cl %CLARGS% %* tscall.c
popd
goto :EOF

:vs_not_installed
echo>&2 Microsoft C/C++ compiler does appear to be installed.
exit /b 1
