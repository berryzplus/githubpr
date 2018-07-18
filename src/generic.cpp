#include "StdAfx.h"
#include "generic.h"

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

// set environment variable.
void setEnvStr(_In_z_ LPCWSTR pszVarName, _In_z_ LPCWSTR pszValue)
{
	// set environment variable.
	if (::_wputenv_s(pszVarName, pszValue))
	{
		THROW_APP_EXCEPTION("can't set environment value.");
	}
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
