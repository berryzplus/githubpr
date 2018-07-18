
#include "StdAfx.h"
#include "PrivateProfile.h"

PrivateProfile::PrivateProfile(_In_ const std::wstring &profileName)
	: _profileName(profileName.c_str())
	, _buffer(0x800, '\0')
{
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
	//moveで抜け殻になる対策
	if (_buffer.size() == 0 && 0 < _buffer.capacity()) {
		_buffer._Eos(1);
		_buffer.at(0) = '\0';
	}
	SIZE_T cbLoad = 0;
	for (;;) {
		
		// INIファイルの項目を取得
		cbLoad = ::GetPrivateProfileString(appName, keyName, pDefault, &*_buffer.begin(), static_cast<DWORD>(_buffer.capacity()), _profileName);

		// メモリが十分に足りていた場合
		if (cbLoad + 1 < _buffer.capacity()) {
			_buffer._Eos(cbLoad);
			break;
		}

		// メモリの再確保を行う
		const SIZE_T cbNewBuffer = _buffer.capacity();
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
