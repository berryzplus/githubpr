#pragma once

class PrivateProfile {
	std::wstring _profileName;

	typedef PrivateProfile _Myt;

public:
	PrivateProfile(_In_ const std::wstring &profileName);
	PrivateProfile() noexcept = default;
	virtual ~PrivateProfile() noexcept = default;
	PrivateProfile(_In_ const _Myt &other) = delete;
	_Myt& operator = (_In_ const _Myt &rhs) = delete;
	std::wstring getString(
		_In_z_ LPCWSTR appName,
		_In_z_ LPCWSTR keyName,
		_In_opt_z_ LPCWSTR pDefault = L"",
		_Inout_opt_ std::wstring* pstrBuffer = NULL
	) const;
	void writeString(
		_In_z_ LPCWSTR appName,
		_In_z_ LPCWSTR keyName,
		_In_z_ LPCWSTR value
	);

	bool valid() const noexcept { return !_profileName.empty(); }
	const std::wstring& GetProfileName() const noexcept { return _profileName; }
	PrivateProfile(_Inout_ _Myt &&other) noexcept;
	_Myt& operator = (_Inout_ _Myt &&rhs) noexcept;
};
