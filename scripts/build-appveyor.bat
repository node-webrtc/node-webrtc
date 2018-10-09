@ECHO OFF
SETLOCAL
SET EL=0

IF /I "%platform%"=="x64" powershell Install-Product node $env:nodejs_version x64
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm install
SET SKIP_DOWNLOAD=true
CALL npm install
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

:CHECK_NPM_TEST_ERRORLEVEL
ECHO npm run lint
CALL npm run lint
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm test
CALL npm test
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm run test:bridge
CALL npm run test:bridge
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
