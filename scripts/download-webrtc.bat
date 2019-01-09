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

ECHO download gn
CALL download_from_google_storage --no_resume --platform=win32 --no_auth --bucket chromium-gn -s src/buildtools/win/gn.exe.sha1
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO lastchange
CALL python src\build\util\lastchange.py -o src\build\util\LASTCHANGE
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO download ciopfs
CALL download_from_google_storage --no_resume --no_auth --bucket chromium-browser-clang/ciopfs -s src\build\ciopfs.sha1
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
