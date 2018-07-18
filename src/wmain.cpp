
#include "StdAfx.h"
#include "GitHubPR.h"

#include "generic.h"


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

