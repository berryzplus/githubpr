
#include "StdAfx.h"
#include "BatchCommand.h"

#include "generic.h"

#ifdef __MINGW32__
#include <ext/stdio_filebuf.h>
#endif /* __MINGW32__ */


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

//バッチを実行してストリームを取得する
std::unique_ptr<std::wistream> BatchCommand::invokeAndGetStream()
{
	if (batName.empty()) {
		THROW_APP_EXCEPTION("batch has no content.");
	}

#ifdef _MSC_VER
	std::wofstream os(batName.c_str());
#else
	std::wofstream os(convertWcsToString(batName.c_str(), batName.length()));
#endif
	os.write(batContent.c_str(), batContent.length());
	os.close();

	FILE* fp = ::_wpopen(batName.c_str(), L"rt");
	if (!fp) {
		THROW_APP_EXCEPTION("batch execution failed.");
	}

#ifdef _MSC_VER
	return std::unique_ptr<std::wistream>(new std::wifstream(fp));
#else
	auto *pfb = new __gnu_cxx::stdio_filebuf<WCHAR>(fp, std::ios_base::in);
	return std::unique_ptr<std::wistream>(new std::wistream((std::basic_filebuf<WCHAR>*)pfb));
#endif
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
	std::unique_ptr<std::wistream> is(invokeAndGetStream());
	std::locale x(std::locale::classic(), mytypeHolder.get());
	is->imbue(x);

	std::istream_iterator<std::wstring, WCHAR> lineIter(*is);
	std::istream_iterator<std::wstring, WCHAR> eos;
	std::list<std::wstring> lines;
	std::copy(lineIter, eos, std::back_inserter(lines));
	is.reset(nullptr);

	return std::move(lines);
}

//後始末
BatchCommand::~BatchCommand() noexcept(false)
{
	if (!batName.empty() && ::PathFileExists(batName.c_str())) {
		const WCHAR format[] = L"del /F /Q %s > NUL 2>&1";
		const size_t requiredLength = ::_scwprintf(format, batName.c_str());
		std::wstring delBatCmd(requiredLength, '\0');
		const size_t length = ::swprintf_s(&*delBatCmd.begin(), delBatCmd.capacity(), format, batName.c_str());
		::_wsystem(delBatCmd.c_str());
	}
}
