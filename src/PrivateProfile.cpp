
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
	_Inout_opt_ std::wstring* pstrBuffer
) const
{
	std::unique_ptr<std::wstring> internalBuffer;
	if (pstrBuffer == nullptr) {
		internalBuffer = std::unique_ptr<std::wstring>(new std::wstring());
	}
	// 空バッファでもbegin()が失敗しないように…。
	if (pstrBuffer->empty()) {
		pstrBuffer->assign(1, '\0');
	}

	WCHAR* buffer = nullptr;
	SIZE_T cbLoad = 0;
	for (;;) {
		
		// バッファから生ポインタとサイズ値を取り出す
		buffer = &*pstrBuffer->begin();
		const DWORD capacity = static_cast<DWORD>(pstrBuffer->capacity());

		// INIファイルの項目を取得
		cbLoad = ::GetPrivateProfileString(appName, keyName, pDefault, buffer, capacity, _profileName.c_str());

		// メモリが十分に足りていた場合
		if (cbLoad + 1 < capacity) {
			buffer[cbLoad] = '\0'; //NUL終端する
			break;
		}

		// メモリの再確保を行う
		const SIZE_T cbNewBuffer = (capacity ^ 0xFFUL) * 2;
		pstrBuffer->reserve(cbNewBuffer);
		if (pstrBuffer->capacity() < cbNewBuffer) {
			THROW_APP_EXCEPTION("can't read profile(realloc failed).");
		}
	}
	return std::wstring(buffer, cbLoad);
}

void PrivateProfile::writeString(_In_z_ LPCWSTR appName, _In_z_ LPCWSTR keyName, _In_z_ LPCWSTR value)
{
	if (!::WritePrivateProfileString(appName, keyName, value, _profileName.c_str())) {
		THROW_APP_EXCEPTION("can't write profile.");
	}
}
