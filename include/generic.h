#pragma once


#define THROW_APP_EXCEPTION(message) \
	throw std::runtime_error(message)


std::wstring loadString(_In_ WORD wStringResourceId);
std::wstring getEnvStr(_In_z_ LPCWSTR pszVarName);
void setEnvStr(_In_z_ LPCWSTR pszVarName, _In_z_ LPCWSTR pszValue);
std::wstring convertMbsToWString(_In_reads_z_(cchMbString) LPCSTR pszMbString, _In_ SIZE_T cchMbString);
std::string convertWcsToString(_In_reads_z_(cchWcString) LPCWSTR pszWcString, _In_ SIZE_T cchWcString);

#ifdef __MINGW32__
template<size_t _MaxCount>
inline
errno_t _wmktemp_s(wchar_t(&_TemplateName)[_MaxCount]) {
	return ::_wmktemp_s(_TemplateName, _MaxCount - 1);
}
#endif /* __MINGW32__ */
