﻿// GitHubPR.h : application main header.

#pragma once

#include "PrivateProfile.h"
#include "BatchCommand.h"

class GitHubPrApp {
	PrivateProfile profile;
	std::wstring gitCommandDir;
	std::wstring gitRemoteName;
	std::wstring homeBranch;
	std::wstring branchPrefix;
	std::wstring prNumber;

public:
	void initSettings(_In_ const std::wstring &targetDir);
	void executeBatch() const;

private:
	std::wstring _initSetting(
		_In_z_ LPCWSTR keyName,
		_In_ std::function<std::wstring()> func,
		_In_opt_ bool skipSave = false
	);
	std::wstring _getGitCommandDir() const;
	std::wstring _getRemoteName() const;
	std::wstring _getBranchPrefix() const;
	std::wstring _getHomeBranch() const;
	std::wstring _getPrNumber() const;
};
