// GitHubPR.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"
#include "GitHubPR.h"

#include "generic.h"


// 定数
const WCHAR GITHUB_PR[] = L"GitHub PR";
const WCHAR GIT_COMMAND_DIR[] = L"GIT_COMMAND_DIR";
const WCHAR COMMAND_DIR_SUFFIX[] = L"\\Git\\cmd";
const WCHAR CONFIG_SUFFIX[] = L"\\.git\\config";
const WCHAR PROFILE_SUFFIX[] = L"\\.git\\github.config";
const WCHAR GIT_REMOTE_NAME[] = L"GIT_REMOTE_NAME";
const WCHAR HOME_BRANCH[] = L"HOME_BRANCH";
const WCHAR BRANCH_PREFIX[] = L"BRANCH_PREFIX";
const WCHAR PR_NUMBER[] = L"PR_NUMBER";


void GitHubPrApp::initSettings(_In_ const std::wstring &targetDir)
{
	::SetConsoleTitle(GITHUB_PR);

	std::wstring curDir(targetDir);
	for (;;) {
		if (::PathFileExists(curDir.c_str())) {
			// カレントディレクトリ変更
			if (::SetCurrentDirectory(curDir.c_str())) {
				break;
			}
		}
		if (!curDir.empty()) {
			std::wcout << loadString(IDS_BAD_ANSWER) << std::endl;
		}
		std::wcout << loadString(IDS_ENTER_GIT_DIR) << std::endl;
		std::getline(std::wcin, curDir);
	}

	std::wstring configName = curDir + CONFIG_SUFFIX;
	if (!::PathFileExists(configName.c_str())) {
		THROW_APP_EXCEPTION("git config not found.");
	}

	std::wstring profileName = curDir + PROFILE_SUFFIX;
	profile = std::move(PrivateProfile(profileName));

	constexpr bool skipSave = true;
	gitCommandDir = _initSetting(GIT_COMMAND_DIR, [this]() {return _getGitCommandDir(); }, skipSave);
	gitRemoteName = _initSetting(GIT_REMOTE_NAME, [this]() {return _getRemoteName(); }, skipSave);
	branchPrefix = _initSetting(BRANCH_PREFIX, [this]() {return _getBranchPrefix(); }, skipSave);
	prNumber = _initSetting(PR_NUMBER, [this]() {return _getPrNumber(); }, skipSave);

	{
		BatchCommand cmd(IDR_CMD_CHECKOUT_BRANCH);
		auto retList = std::move(cmd.invokeAndGetLines());
		currentBranch = std::move(retList.back());
	}
	if (!currentBranch.empty()) {
		setEnvStr(L"CURRENT_BRANCH", currentBranch.c_str());
		homeBranch = _initSetting(HOME_BRANCH, [this]() {return _getHomeBranch(); }, skipSave);
	}
}

std::wstring GitHubPrApp::_initSetting(
	_In_z_ LPCWSTR keyName,
	_In_ std::function<std::wstring()> func,
	_In_opt_ bool skipSave
)
{
	// get environment variable.
	std::wstring setting = getEnvStr(keyName);
	if (setting.empty()) {
		// get profile variable.
		setting = profile.getString(GITHUB_PR, keyName);
		if (setting.empty()) {
			setting = std::move(func());
			if (!skipSave) {
				profile.writeString(GITHUB_PR, keyName, setting.c_str());
			}
		}
		setEnvStr(keyName, setting.c_str());
	}
	return std::move(setting);
}

std::wstring GitHubPrApp::_getGitCommandDir() const
{
	std::wstring programFiles = getEnvStr(L"ProgramW6432");
	if (programFiles.empty()) {
		programFiles = getEnvStr(L"ProgramFiles");
		if (programFiles.empty()) {
			THROW_APP_EXCEPTION("env[ProgramFiles] not found.");
		}
	}
	std::wstring gitCommandDir = programFiles + COMMAND_DIR_SUFFIX;
	if (!::PathFileExists(gitCommandDir.c_str())) {
		THROW_APP_EXCEPTION("GNU Git not found.");
	}
	std::wstring path = getEnvStr(L"PATH");
	if (0 != ::_wcsnicmp(gitCommandDir.c_str(), path.c_str(), gitCommandDir.length())) {
		path = gitCommandDir + L';' + path;
		setEnvStr(L"PATH", path.c_str());
	}
	return std::move(gitCommandDir);
}

std::wstring GitHubPrApp::_getRemoteName() const
{
	std::list<std::wstring> remotes;
	{
		BatchCommand cmd(IDR_CMD_ENUM_REMOTES);
		remotes = cmd.invokeAndGetLines();
		remotes.remove(L"");
	}
	if (remotes.empty()) {
		THROW_APP_EXCEPTION("repository has no remote.");
	}
	if (remotes.size() == 1) {
		return std::move(remotes.front());
	}

	std::vector<std::wstring> remoteVector(remotes.cbegin(), remotes.cend());
	int ans = 0;
	for (;;) {
		std::wcout << loadString(IDS_ENTER_REMOTE) << std::endl;
		int nIndex = 1;
		std::for_each(remoteVector.cbegin(), remoteVector.cend(),
			[&nIndex](const std::wstring &remote) {
			std::wcout << '\t' << nIndex++ << ':' << remote << std::endl;
		});
		std::wcout << std::endl;
		std::wcin >> ans;
		std::wcin.get();
		std::wcout << std::endl;
		if (0 < ans && ans <= static_cast<int>(remotes.size())) {
			break;
		}
		std::wcout << loadString(IDS_BAD_ANSWER) << std::endl;
	}
	return std::move(remoteVector.at(ans - 1));
}

std::wstring GitHubPrApp::_getBranchPrefix() const
{
	return L"pull-request";
}

std::wstring GitHubPrApp::_getPrNumber() const
{
	int ans = 0;
	for (;;) {
		std::wcout << loadString(IDS_ENTER_PRNUMBER) << std::endl;
		std::wcin >> ans;
		std::wcin.get();
		if (0 < ans) {
			break;
		}
		std::wcout << loadString(IDS_BAD_ANSWER) << std::endl;
	}
	std::wstring prNumber(std::to_wstring(ans));
	std::wstring branchPrefix = getEnvStr(BRANCH_PREFIX);
	std::wstring branchName = branchPrefix + L'/' + prNumber;
	setEnvStr(L"BRANCH_NAME", branchName.c_str());
	return std::move(prNumber);
}

std::wstring GitHubPrApp::_getHomeBranch() const
{
	std::list<std::wstring> branchs;
	{
		BatchCommand cmd(IDR_CMD_ENUM_BRANCHS);
		branchs = std::move(cmd.invokeAndGetLines());
		branchs.remove(L"");
		std::wregex reBranch(L"^\\*?\\s+(.+)$");
		std::wsmatch match;
		std::for_each(branchs.begin(), branchs.end(), [&reBranch, &match](_Inout_ std::wstring &branch) {
			if (std::regex_match(branch, match, reBranch)) {
				branch = match[1];
			}
		});
	}
	if (branchs.empty()) {
		THROW_APP_EXCEPTION("repository has no branch.");
	}
	if (branchs.size() == 1) {
		return std::move(branchs.front());
	}
	else if (std::find(branchs.cbegin(), branchs.cend(), L"master") != branchs.cend()) {
		return L"master";
	}

	int ans = 0;
	std::vector<std::wstring> branchVector(branchs.cbegin(), branchs.cend());
	for (;;) {
		std::wcout << loadString(IDS_ENTER_BRANCH) << std::endl;
		int nIndex = 1;
		std::for_each(branchVector.cbegin(), branchVector.cend(),
			[&nIndex](const std::wstring &branch) {
			std::wcout << '\t' << nIndex++ << ':' << branch << std::endl;
		});
		std::wcout << std::endl;
		std::wcin >> ans;
		std::wcin.get();
		std::wcout << std::endl;
		if (0 < ans && ans <= static_cast<int>(branchs.size())) {
			break;
		}
		std::wcout << loadString(IDS_BAD_ANSWER) << std::endl;
	}
	return std::move(branchVector.at(ans - 1));
}

void GitHubPrApp::executeBatch() const
{
	//バッチファイルを実行する
	BatchCommand cmd(IDR_CMD_GET_PR);
	std::list<std::wstring> lines(cmd.invokeAndGetLines());
	std::for_each(lines.cbegin(), lines.cend(), [](const std::wstring &line) {std::wcout << line << std::endl; });
}
