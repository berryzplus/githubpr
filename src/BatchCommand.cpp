
#include "StdAfx.h"
#include "BatchCommand.h"

#include "generic.h"

#include <strsafe.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#ifdef __MINGW32__
#include <ext/stdio_filebuf.h>
#endif /* __MINGW32__ */

_Check_return_
inline FILE* my_popen(
	_In_z_ wchar_t const* _Command,
	_In_z_ wchar_t const* _Mode
) noexcept;

_Check_return_
inline FILE* my_popen_internal(
	_In_z_ wchar_t const* _ExeName,
	_In_z_ wchar_t const* _Command,
	_In_z_ wchar_t const* _Mode
) noexcept;

class commandline_args_iterator : public std::iterator<std::input_iterator_tag, LPCWSTR>
{
	typedef std::iterator<std::input_iterator_tag, LPCWSTR>		_Mybase;
	typedef commandline_args_iterator							_Myt;

private:
	std::wstring	m_strToken;		//!< 分割対象文字列(コピー)
	std::wstring	m_strDelimit;	//!< デリミタ文字列(コピー)
	WCHAR*			m_pToken;		//!< 切り出されたトークン
	const WCHAR*	m_pEndOfToken;	//!< 分割対象文字列の終端

public:
	/*!
	 * @brief 引数ありコンストラクタ(検索開始用)
	 *
	 * @param [in]		szToken		分割対象文字列。
	 * @param [in,opt]	szDelimit	デリミタ文字列。
	 *								省略した場合、空白区切りで分割する。
	 */
	commandline_args_iterator(
		_In_z_ LPCWSTR szToken,
		_In_opt_z_ LPCWSTR szDelimit = L" ")
		: m_strToken(szToken ? szToken : L"")
		, m_strDelimit(szDelimit)
		, m_pToken(const_cast<WCHAR*>(m_strToken.c_str()))
		, m_pEndOfToken(m_pToken + m_strToken.length())
	{
		if (!m_strToken.empty())
		{
			tokenize();
		}
	}
	//引数なしコンストラクタ(end構築用)
	commandline_args_iterator(void)
		: m_pToken(nullptr)
		, m_pEndOfToken(nullptr)
	{
	}
	commandline_args_iterator(const _Myt& rhs) = delete;
	_Myt& operator = (const _Myt& rhs) = delete;

public:
	//前置インクリメント
	_Myt& operator++(void)
	{
		//検索失敗(or 検索完了後)には呼び出さない
		if (!m_pToken)
		{
			return (*this);
		}
		//文字列＋１の位置に進める
		m_pToken += ::wcslen(m_pToken) + 1;
		tokenize();
		return (*this);
	}
	//後置インクリメントは実装しない
	//_Myt operator++( int );
	LPCWSTR   operator *  (void) const { return m_pToken; }
	bool      operator == (const _Myt& rhs) const { return m_pToken == rhs.m_pToken; }
	bool      operator != (const _Myt& rhs) const { return !(*this == rhs); }

protected:
	void tokenize(void) noexcept
	{
		auto beg = m_strDelimit.cbegin();
		auto end = m_strDelimit.cend();

		//先頭の区切り文字をスキップする
		for (; m_pToken < m_pEndOfToken; ++m_pToken)
		{
			WCHAR* p = nullptr;
			for (auto iter = beg; iter != end; ++iter) {
				if (m_pToken[0] != *iter) continue;
				p = m_pToken;
				break;
			}
			if (!p) break;
		}

		if (m_pToken == m_pEndOfToken)
		{
			m_pToken = NULL; //次のトークンは見つからなかった
			return;
		}

		//文字列がダブルクォーテーションで始まっているかチェック
		if (m_pToken[0] == L'\"')
		{
			//文字列ポインタを進める
			m_pToken++;
			//閉じクォーテーションを探す
			for (auto p = m_pToken; p < m_pEndOfToken; p++)
			{
				if (p[0] != L'\"') continue;
				// 特殊ケース:連続するクォーテーション "" はスキップする
				if (p + 1 < m_pEndOfToken && p[1] == L'\"') {
					p++;
					continue;
				}
				*p = '\0'; //NUL終端する
				return;
			}
		}

		//次の区切り文字を探す
		for (auto p = m_pToken; p < m_pEndOfToken; p++)
		{
			for (auto iter = beg; iter != end; ++iter) {
				if (p[0] != *iter) continue;
				*p = '\0'; //NUL終端する
				return;
			}
		}
	}
};

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

	std::wstring cmdStr;

	std::wregex reEcho(L"^@?echo\s+off", std::regex_constants::icase);
	if (std::regex_match(batContent, reEcho)) {
#ifdef _MSC_VER
		std::wofstream os(batName.c_str());
#else
		std::wofstream os(convertWcsToString(batName.c_str(), batName.length()));
#endif
		os.write(batContent.c_str(), batContent.length());
		os.close();
		cmdStr = batName;
	}
	else {
		cmdStr = batContent;
	}

	FILE* fp = my_popen(cmdStr.c_str(), L"rt");
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

_Check_return_
inline FILE* my_popen(
	_In_z_ wchar_t const* _Command,
	_In_z_ wchar_t const* _Mode
) noexcept
{
	// コマンドラインを解析する
	commandline_args_iterator iter(_Command);
	commandline_args_iterator end;
	if (iter == end) return NULL;

	// コマンドを取り出す
	std::wstring cmd;
	if (::PathIsRelative(*iter)) {
		WCHAR szTemp[MAX_PATH] = L"";
		// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365527(v=vs.85).aspx
		DWORD dwRet = ::SearchPath(NULL, *iter, L".exe", _countof(szTemp), szTemp, nullptr);
		if (dwRet < _countof(szTemp)) {
			cmd = szTemp;
		}
	}
	else {
		cmd = *iter;
	}

	// 存在チェック
	auto nRet = ::_waccess_s(cmd.c_str(), 4);

	FILE* fp = NULL;
	if (nRet) {
		// 標準の popen を呼び出す
		fp = ::_wpopen(_Command, _Mode);
	}
	else {
		// 独自の popen を呼び出す
		fp = my_popen_internal(
			cmd.c_str(),
			_Command,
			_Mode
		);
	}
	return fp;
}

_Check_return_
inline FILE* my_popen_internal(
	_In_z_ wchar_t const* _ExeName,
	_In_z_ wchar_t const* _Command,
	_In_z_ wchar_t const* _Mode
) noexcept
{
	if (_Mode[0] == '\0') return nullptr;

	enum { STDIN, STDOUT };
	int const std_fh = _Mode[0] == 'w' ? STDIN : STDOUT;

	int pipe_handle = 0;
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	{
		int pipe_mode = _O_NOINHERIT;
		if (_Mode[1] == 't') { pipe_mode |= _O_TEXT; }
		if (_Mode[1] == 'b') { pipe_mode |= _O_BINARY; }

		int pipe_handles[2];
		if (_pipe(pipe_handles, 1024, pipe_mode) == -1)
			return nullptr;

		int ordered_pipe_handles[] =
		{
			std_fh == STDIN ? pipe_handles[0] : pipe_handles[1],
			std_fh == STDIN ? pipe_handles[1] : pipe_handles[0]
		};

		auto process_handle = ::GetCurrentProcess();
		if (!DuplicateHandle(
			process_handle,
			reinterpret_cast<HANDLE>(_get_osfhandle(ordered_pipe_handles[0])),
			process_handle,
			&hHandle,
			0,
			TRUE,
			DUPLICATE_SAME_ACCESS))
		{
			return nullptr;
		}

		::_close(ordered_pipe_handles[0]);
		ordered_pipe_handles[0] = -1;
		pipe_handle = ordered_pipe_handles[1];
	}

	std::wstring cmdline(_Command);

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = STDIN ? hHandle : ::GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = STDIN ? ::GetStdHandle(STD_OUTPUT_HANDLE) : hHandle;
	si.hStdError = STDIN ? ::GetStdHandle(STD_ERROR_HANDLE) : hHandle;

	//コマンドライン実行
	PROCESS_INFORMATION pi;
	auto bStatus = ::CreateProcessW(
		_ExeName,				// lpApplicationName
		&*cmdline.begin(),		// lpCommandLine
		NULL,					// lpProcessAttributes
		NULL,					// lpThreadAttributes
		TRUE,					// bInheritHandles
		CREATE_NEW_CONSOLE
		| CREATE_UNICODE_ENVIRONMENT
		| HIGH_PRIORITY_CLASS
		,						// dwCreationFlags
		NULL,					// lpEnvironment
		NULL,					// lpCurrentDirectory
		&si,
		&pi);

	if (!bStatus) return nullptr;

	::CloseHandle(pi.hThread);
	::CloseHandle(pi.hProcess);

	::CloseHandle(hHandle);

	FILE* return_value = ::_wfdopen(pipe_handle, _Mode);
	return return_value;
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
