@rem echo off
IF "%CURRENT_BRANCH%" == "" (
  %GIT% fetch %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head:%BRANCH_NAME%
  %GIT% checkout %BRANCH_NAME%
) ELSE (
  %GIT% checkout %HOME_BRANCH% > NUL 2>&1
  %GIT% checkout %BRANCH_NAME%
  %GIT% pull %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head
)
