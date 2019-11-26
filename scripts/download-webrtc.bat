@ECHO OFF
SET EL=0

ECHO Add depot_tools to PATH
set PATH=%DEPOT_TOOLS%;%PATH%
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO SET DEPOT_TOOLS_WIN_TOOLCHAIN=0
SET DEPOT_TOOLS_WIN_TOOLCHAIN=0
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO gclient config
CALL gclient config --unmanaged --spec solutions=[{\"name\":\"src\",\"url\":\"https://webrtc.googlesource.com/src.git\"}]
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO gclient sync
CALL gclient sync --shallow --no-history --nohooks --with_branch_heads -r %WEBRTC_REVISION% -R
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO lastchange
CALL python src\build\util\lastchange.py -o src\build\util\LASTCHANGE
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO update toolchain
CALL python src\build\vs_toolchain.py update --force
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO download clang
CALL python src\tools\clang\scripts\update.py
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO rmdir webrtc
rmdir webrtc

ECHO mklink /D webrtc src
mklink /D webrtc src
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE

EXIT /b %EL%
