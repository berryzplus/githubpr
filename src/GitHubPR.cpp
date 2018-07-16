// GitHubPR.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"
#include "GitHubPR.h"


#pragma comment(lib, "Shlwapi.lib")

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


int wmain(int argc, WCHAR* argv[])
{
	GitHubPrApp app;
	try {
		// 最初の引数を取り出す
		LPCWSTR arg = (1 < argc && argv[1][0] != '\0') ? argv[1] : L"";

		// 初期化する
		app.initSettings(arg);

		//バッチファイルを実行する
		app.executeBatch();

		//ユーザ入力を待機する(読み捨て)
		std::wcout << loadString(IDS_PRESS_ANY_KEY) << std::endl;
		std::wcin.get();
	}
	catch (const std::runtime_error &e) {
		const char* what = e.what();
		const size_t cbWhat = ::strlen(what);
		::MessageBox(NULL, convertMbsToWString(what, cbWhat).c_str(), GITHUB_PR, MB_OK);
	}

	return 0;
}

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
	gitCommandDir = _initSetting(GIT_COMMAND_DIR, [this]() {return _getGitCommandDir(); });
	gitRemoteName = _initSetting(GIT_REMOTE_NAME, [this]() {return _getRemoteName(); });
	branchPrefix = _initSetting(BRANCH_PREFIX, [this]() {return _getBranchPrefix(); });
	homeBranch = _initSetting(HOME_BRANCH, [this]() {return _getHomeBranch(); });
	prNumber = _initSetting(PR_NUMBER, [this]() {return _getPrNumber(); }, skipSave);
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
		// set environment variable.
		if (::_wputenv_s(keyName, setting.c_str()))
		{
			THROW_APP_EXCEPTION("can't set environment value.");
		}
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
	return std::to_wstring(ans);
}

void GitHubPrApp::executeBatch() const
{
	//バッチファイルを実行する
	BatchCommand cmd(IDR_CMD_GET_PR);
	std::list<std::wstring> lines(cmd.invokeAndGetLines());
	std::for_each(lines.cbegin(), lines.cend(), [](const std::wstring &line) {std::wcout << line << std::endl; });
}

// get current directory.
std::wstring getCurrentDirectory()
{
	const DWORD requiredSize = ::GetCurrentDirectory(0, nullptr);
	std::wstring curDir(requiredSize, '\0');
	const DWORD nRet = ::GetCurrentDirectory(requiredSize, &*curDir.begin());
	curDir._Eos(nRet);
	return std::move(curDir);
}

// get environment variable.
std::wstring getEnvStr(_In_z_ LPCWSTR pszVarName)
{
	size_t requiredSize = 0;
	::_wgetenv_s(&requiredSize, NULL, 0, pszVarName);
	if (0 < requiredSize)
	{
		std::wstring varStr(requiredSize - 1, '\0');
		::_wgetenv_s(&requiredSize, &*varStr.begin(), requiredSize, pszVarName);
		if (0 < requiredSize)
		{
			varStr._Eos(requiredSize - 1);
			return std::move(varStr);
		}
	}
	return std::wstring();
}

// load string from resource.
std::wstring loadString(_In_ WORD wStringResourceId)
{
	//文字列の実体は文字列テーブルとしてバンドルされるので、
	//wStringResourceIdIdをバンドルIDに変換し、リソース検索する。
	//詳細についてはMSDN FindResourceEx関数の説明を参照。
	//この処理はLoadString関数内部で行われているものと同じ。
	const WORD wBundleId = wStringResourceId / 16 + 1;
	const WORD wBundleOffset = wStringResourceId & 0x0F;
	HRSRC hres = ::FindResource(NULL, MAKEINTRESOURCE(wBundleId), RT_STRING);
	if (!hres) {
		THROW_APP_EXCEPTION("resource not found.");
	}

	//文字列テーブルを読み込む
	HGLOBAL hResData = ::LoadResource(NULL, hres);
	if (!hResData)
	{
		THROW_APP_EXCEPTION("load resource has failed.");
	}

	//戻り値を用意する
	std::wstring content;

	//文字列テーブルをロックする
	LPCWSTR pResData = static_cast<LPWSTR>(::LockResource(hResData));
	if (pResData)
	{
		//文字列テーブルの先頭が返るのでwStringResourceIdのところまでポインタを進める
		LPCWSTR pwRsrc = pResData;
		for (WORD i = 0; i < wBundleOffset; i++)
		{
			pwRsrc = &pwRsrc[*pwRsrc + 1];
		}

		//文字列をコピーして戻り値を準備する
		content = std::wstring(&pwRsrc[1], *pwRsrc);

		//ロックを解除する
		//※↓の名前のWindowsAPIは存在しない。
		//　LockするのにUnlockしないのは奇妙なので、形式的に記述しておく。
		UnlockResource(pResData);
	}

	//開放処理を行う
	//※実行モジュールは仮想メモリ空間に展開されたままなので、
	//　取得した文字列ポインタはそのまま利用しつづけて問題ない。
	::FreeResource(hResData);

	if (!pResData)
	{
		THROW_APP_EXCEPTION("lock resource has failed.");
	}

	return std::move(content);
}

// convert multi-byte string to wide string.
std::wstring convertMbsToWString(_In_reads_z_(cchMbString) LPCSTR pszMbString, _In_ SIZE_T cchMbString)
{
	if (INT_MAX < cchMbString) {
		throw std::invalid_argument("cchMbString is too large.");
	}
	const int nwChars = ::MultiByteToWideChar(
		CP_ACP, 0,
		pszMbString, static_cast<int>(cchMbString),
		nullptr, 0
	);
	if (0 == nwChars) {
		return std::wstring();
	}
	std::wstring content(nwChars, '\0');
	const int nwRet = ::MultiByteToWideChar(
		CP_ACP, 0,
		pszMbString, static_cast<int>(cchMbString),
		&*content.begin(), static_cast<int>(content.capacity())
	);
	content._Eos(nwRet);
	return std::move(content);
}
