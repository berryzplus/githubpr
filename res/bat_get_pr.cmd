@echo off
path=%GIT_COMMAND_DIR%;%path%

set BRANCH_NAME=%BRANCH_PREFIX%/%PR_NUMBER%

git stash > NUL 2>&1
git checkout %HOME_BRANCH% > NUL 2>&1
git branch -D %BRANCH_NAME% > NUL 2>&1

git fetch %GIT_REMOTE_NAME% pull/%PR_NUMBER%/head:%BRANCH_NAME%
git checkout %BRANCH_NAME%
