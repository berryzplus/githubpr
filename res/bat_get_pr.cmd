@echo off
IF "%CURRENT_BRANCH%" == "" (
  git fetch %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head:%BRANCH_NAME%
  git checkout %BRANCH_NAME%
) ELSE (
  git checkout %HOME_BRANCH% > NUL 2>&1
  git checkout %BRANCH_NAME%
  git pull %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head
)
