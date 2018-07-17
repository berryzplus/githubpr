@echo off
git checkout %BRANCH_NAME%
IF ERRORLEVEL 1 (
  git fetch %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head:%BRANCH_NAME%
  git checkout %BRANCH_NAME%
) ELSE (
  git pull %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head
)
