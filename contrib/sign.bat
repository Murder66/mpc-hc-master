@ECHO OFF
REM (C) 2013 see Authors.txt
REM
REM This file is part of MPC-HC.
REM
REM MPC-HC is free software; you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation; either version 3 of the License, or
REM (at your option) any later version.
REM
REM MPC-HC is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with this program.  If not, see <http://www.gnu.org/licenses/>.


SETLOCAL

IF "%~1" == "" (
  ECHO %~nx0: No input specified!
  SET SIGN_ERROR=True
  GOTO END
)

IF NOT DEFINED VS120COMNTOOLS (
  ECHO %~nx0: Visual Studio 2013 does not seem to be installed...
  SET SIGN_ERROR=True
  GOTO END
)

IF NOT EXIST "%~dp0..\signinfo.txt" (
  ECHO %~nx0: %~dp0..\signinfo.txt is not present!
  SET SIGN_ERROR=True
  GOTO END
)

SET SIGN_CMD=
SET /P SIGN_CMD=<%~dp0..\signinfo.txt

TITLE Signing "%~1"...
ECHO. & ECHO Signing "%~1"...

signtool /? 2>NUL || CALL "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" 2>NUL
IF %ERRORLEVEL% NEQ 0 (
  ECHO vcvarsall.bat call failed.
  GOTO End
)

REM Repeat n times when signing fails
SET REPEAT=3
SET TRY=0

:SIGN
signtool sign %SIGN_CMD% "%~1"
SET /A TRY+=1
IF %ERRORLEVEL% NEQ 0 (
  IF TRY LSS REPEAT (
    REM Wait 5 seconds before next try
    PING -n 5 127.0.0.1 >NUL
    GOTO SIGN
  )
  SET SIGN_ERROR=True
  GOTO END
)


:END
IF /I "%SIGN_ERROR%" == "True" (
  IF "%~1" == "" PAUSE
  ENDLOCAL
  EXIT /B 1
)
ENDLOCAL
EXIT /B
