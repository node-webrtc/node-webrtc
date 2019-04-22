@ECHO OFF
SETLOCAL
SET EL=0

ECHO Install Node %nodejs_version%
powershell Install-Product node $env:nodejs_version x64
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm test
CALL npm test
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

ECHO npm run test:browsers
CALL npm run test:browsers
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO DONE

:ERROR
ECHO ERRORLEVEL^: %ERRORLEVEL%
SET EL=%ERRORLEVEL%

:DONE

EXIT /b %EL%
