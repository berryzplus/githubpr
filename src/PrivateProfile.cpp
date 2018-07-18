
#include "StdAfx.h"
#include "PrivateProfile.h"

PrivateProfile::PrivateProfile(_In_ const std::wstring &profileName)
	: _profileName(profileName.c_str())
	, _buffer(0x100, '\0')
{
	//初期値をセットする
	_buffer._Eos(0x100);
}

PrivateProfile::PrivateProfile(_Inout_ _Myt &&other) noexcept
	: _profileName(NULL)
	, _buffer()
{
	*this = std::move(other);
}

PrivateProfile& PrivateProfile::operator = (_Inout_ _Myt &&rhs) noexcept
{
	std::swap(_profileName, rhs._profileName);
	std::swap(_buffer, rhs._buffer);
	return *this;
}

std::wstring PrivateProfile::getString(_In_z_ LPCWSTR appName, _In_z_ LPCWSTR keyName, _In_opt_z_ LPCWSTR pDefault)
{
	SIZE_T cbLoad = 0;
	for (;;) {
		
		// バッファから生ポインタとサイズ値を取り出す
		WCHAR* buffer = &*_buffer.begin();
		DWORD capacity = static_cast<DWORD>(_buffer.capacity());

		// INIファイルの項目を取得
		cbLoad = ::GetPrivateProfileString(appName, keyName, pDefault, buffer, capacity, _profileName);

		// メモリが十分に足りていた場合
		if (cbLoad + 1 < capacity) {
			buffer[cbLoad] = '\0'; //NUL終端する
			break;
		}

		// メモリの再確保を行う
		const SIZE_T cbNewBuffer = (capacity ^ 0xFFUL) * 2;
		_buffer.reserve(cbNewBuffer);
		if (_buffer.capacity() < cbNewBuffer) {
			THROW_APP_EXCEPTION("can't read profile(realloc failed).");
		}
	}
	return std::wstring(_buffer, 0, cbLoad);
}

void PrivateProfile::writeString(_In_z_ LPCWSTR appName, _In_z_ LPCWSTR keyName, _In_z_ LPCWSTR value)
{
	if (!::WritePrivateProfileString(appName, keyName, value, _profileName)) {
		THROW_APP_EXCEPTION("can't write profile.");
	}
}
