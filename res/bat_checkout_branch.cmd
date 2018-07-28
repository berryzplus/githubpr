@echo off
git checkout %BRANCH_NAME% > NUL 2>&1
IF ERRORLEVEL 1 (
) ELSE (
  ECHO %BRANCH_NAME%
)
