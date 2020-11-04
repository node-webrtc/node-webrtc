@ECHO OFF
SETLOCAL
SET EL=0

powershell Install-Product node $env:nodejs_version x64
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm install
SET SKIP_DOWNLOAD=true
CALL npm install
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm run lint
CALL npm run lint
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

@REM ECHO Test using Node 8
@REM SET nodejs_version=8
@REM CALL scripts\run-tests.bat
@REM IF %ERRORLEVEL% NEQ 0 GOTO ERROR

@REM ECHO Test using Node 10
@REM SET nodejs_version=10
@REM CALL scripts\run-tests.bat
@REM IF %ERRORLEVEL% NEQ 0 GOTO ERROR

@REM ECHO Test using Node 11
@REM SET nodejs_version=11
@REM CALL scripts\run-tests.bat
@REM IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO Test using Node 12
SET nodejs_version=12
CALL scripts\run-tests.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO Test using Node 13
SET nodejs_version=13
CALL scripts\run-tests.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO Test using Node 14
SET nodejs_version=14
CALL scripts\run-tests.bat
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO Test Electron 4
CALL npm install --no-save electron@4.x
CALL npm run test:electron
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO Test Electron 5
CALL npm install --no-save electron@5.x
CALL npm run test:electron
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO node-pre-gyp package
CALL node_modules\.bin\node-pre-gyp package
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

SET msvs_version=2017
IF NOT "%APPVEYOR_REPO_COMMIT_MESSAGE%" == "%APPVEYOR_REPO_COMMIT_MESSAGE:[publish binary]=%" (ECHO node-pre-gyp publish && CALL node_modules\.bin\node-pre-gyp --msvs_version=%msvs_version% publish)
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE

EXIT /b %EL%
