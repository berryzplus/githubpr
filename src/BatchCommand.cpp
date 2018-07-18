
#include "StdAfx.h"
#include "BatchCommand.h"

#include "generic.h"


BatchCommand::BatchCommand(_In_ WORD wBatchResourceId)
	: wID(wBatchResourceId)
{
	batName = generateBatchName();
	generateBatchFile();
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

// バッチファイルを生成する
void BatchCommand::generateBatchFile()
{
	HRSRC hres = ::FindResource(NULL, MAKEINTRESOURCE(wID), (LPCWSTR)L"BatchCommands");
	if (!hres) {
		THROW_APP_EXCEPTION("resource not found.");
	}
	const SIZE_T cbResData = ::SizeofResource(NULL, hres);
	HGLOBAL hResData = ::LoadResource(NULL, hres);
	if (!hResData) {
		THROW_APP_EXCEPTION("lock resource failed.");
	}
	LPCSTR pResData = static_cast<LPSTR>(::LockResource(hResData));
	std::wstring batchContent(convertMbsToWString(pResData, cbResData));
	UnlockResource(hResData);

	std::wofstream os(batName);
	os.write(batchContent.c_str(), batchContent.length());
	os.close();
}

//バッチを実行してストリームを取得する
std::wifstream BatchCommand::invokeAndGetStream()
{
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
	// del /F /Q %1$s > NUL 2>&1
	const std::wstring format(loadString(IDS_DEL_CMD));
	if (!format.empty() && !batName.empty() && ::PathFileExists(batName.c_str())) {
		const size_t requiredLength = ::_scwprintf_p(format.c_str(), batName.c_str());
		std::wstring delBatCmd(requiredLength, '\0');
		const size_t length = ::_swprintf_p(&*delBatCmd.begin(), delBatCmd.capacity(), format.c_str(), batName.c_str());
		delBatCmd._Eos(length);
		::_wsystem(delBatCmd.c_str());
	}
}
