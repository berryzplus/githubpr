
#include "StdAfx.h"
#include "PrivateProfile.h"

#include "generic.h"


PrivateProfile::PrivateProfile(_In_ const std::wstring &profileName)
	: _profileName(profileName)
{
}

PrivateProfile::PrivateProfile(_Inout_ _Myt &&other) noexcept
{
	*this = std::move(other);
}

PrivateProfile& PrivateProfile::operator = (_Inout_ _Myt &&rhs) noexcept
{
	std::swap(_profileName, rhs._profileName);
	return *this;
}

std::wstring PrivateProfile::getString(
	_In_z_ LPCWSTR appName,
	_In_z_ LPCWSTR keyName,
	_In_opt_z_ LPCWSTR pDefault,
	_Inout_opt_ std::wstring &buffer
) const
{
	// 空バッファでもbegin()が失敗しないように…。
	if (buffer.empty()) {
		buffer.assign(1, '\0');
	}

	SIZE_T cbLoad = 0;
	for (;;) {
		
		// バッファから生ポインタとサイズ値を取り出す
		WCHAR* pszBuffer = &*buffer.begin();
		const DWORD capacity = static_cast<DWORD>(buffer.capacity());

		// INIファイルの項目を取得
		cbLoad = ::GetPrivateProfileString(appName, keyName, pDefault, pszBuffer, capacity, _profileName.c_str());

		// メモリが十分に足りていた場合
		if (cbLoad + 1 < capacity) {
			buffer._Eos(cbLoad); //NUL終端する
			break;
		}

		// メモリの再確保を行う
		const SIZE_T cbNewBuffer = (capacity ^ 0xFFUL) * 2;
		buffer.reserve(cbNewBuffer);
		if (buffer.capacity() < cbNewBuffer) {
			THROW_APP_EXCEPTION("can't read profile(realloc failed).");
		}
	}
	return std::wstring(buffer, 0, cbLoad);
}

void PrivateProfile::writeString(_In_z_ LPCWSTR appName, _In_z_ LPCWSTR keyName, _In_z_ LPCWSTR value)
{
	if (!::WritePrivateProfileString(appName, keyName, value, _profileName.c_str())) {
		THROW_APP_EXCEPTION("can't write profile.");
	}
}
