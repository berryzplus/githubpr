
#include "StdAfx.h"
#include "BatchCommand.h"

#include "generic.h"


BatchCommand::BatchCommand(_In_ WORD wBatchResourceId)
{
	batContent = loadBatchContent(wBatchResourceId);
	batName = generateBatchName();
}

BatchCommand::BatchCommand(_In_opt_ const std::wstring& batchContent)
{
	batContent = std::move(batchContent);
	if (!batContent.empty()) {
		batName = generateBatchName();
	}
}

// バッチ名を生成する
std::wstring BatchCommand::generateBatchName()
{
	WCHAR fnTemplate[MAX_PATH] = { 0 };
	::wcscpy_s(fnTemplate, L"prXXXXXX");
	::_wmktemp_s(fnTemplate);
	::wcscat_s(fnTemplate, L".cmd");
	return std::wstring(fnTemplate);
}

// リソースからバッチ内容を読み込む
std::wstring BatchCommand::loadBatchContent(_In_ WORD wBatchResourceId)
{
	HRSRC hres = ::FindResource(NULL, MAKEINTRESOURCE(wBatchResourceId), (LPCWSTR)L"BatchCommands");
	if (!hres) {
		THROW_APP_EXCEPTION("resource not found.");
	}
	const SIZE_T cbResData = ::SizeofResource(NULL, hres);
	HGLOBAL hResData = ::LoadResource(NULL, hres);
	if (!hResData) {
		THROW_APP_EXCEPTION("load resource failed.");
	}
	LPCSTR pResData = static_cast<LPSTR>(::LockResource(hResData));
	if (!pResData) {
		THROW_APP_EXCEPTION("lock resource failed.");
	}
	std::wstring batchContent(convertMbsToWString(pResData, cbResData));
	UnlockResource(hResData);

	return std::move(batchContent);
}

//バッチを実行してストリームを取得する
std::wifstream BatchCommand::invokeAndGetStream()
{
	if (batName.empty()) {
		THROW_APP_EXCEPTION("batch has no content.");
	}

	std::wofstream os(batName);
	os.write(batContent.c_str(), batContent.length());
	os.close();

	FILE* fp = ::_wpopen(batName.c_str(), L"rt");
	if (!fp) {
		THROW_APP_EXCEPTION("batch execution failed.");
	}
	return std::wifstream(fp);
}

std::list<std::wstring> BatchCommand::invokeAndGetLines()
{
	//istream_iteratorでの文字列取得を行単位にするためのファセット定義
	class wctypeLinemode : public std::ctype<WCHAR>
	{
	public:
		wctypeLinemode(size_t refs = 0)
			: std::ctype<WCHAR>(refs)
		{
		}
		bool do_is(mask _Maskval, WCHAR _Ch) const override
		{
			auto skipThisChar = std::ctype<WCHAR>::do_is(_Maskval, _Ch);
			if (skipThisChar && _Ch != '\r' && _Ch != '\n') {
				return false; //改行文字以外はスキップしない
			}
			return skipThisChar;
		}
	};
	std::unique_ptr<wctypeLinemode> mytypeHolder(std::make_unique<wctypeLinemode>(1));
	std::wifstream is(invokeAndGetStream());
	std::locale x(std::locale::classic(), mytypeHolder.get());
	is.imbue(x);

	std::istream_iterator<std::wstring, WCHAR> lineIter(is);
	std::istream_iterator<std::wstring, WCHAR> eos;
	std::list<std::wstring> lines;
	std::copy(lineIter, eos, std::back_inserter(lines));
	is.close();

	return std::move(lines);
}

//後始末
BatchCommand::~BatchCommand() noexcept(false)
{
	if (!batName.empty() && ::PathFileExists(batName.c_str())) {
		const WCHAR format[] = L"del /F /Q %1$s > NUL 2>&1";
		const size_t requiredLength = ::_scwprintf_p(format, batName.c_str());
		std::wstring delBatCmd(requiredLength, '\0');
		const size_t length = ::_swprintf_p(&*delBatCmd.begin(), delBatCmd.capacity(), format, batName.c_str());
		delBatCmd._Eos(length);
		::_wsystem(delBatCmd.c_str());
	}
}
