#include "StdAfx.h"
#include "PrivateProfile.h"

#include "generic.h"

namespace test {


class PrivateProfileTest : public ::testing::Test {
protected:
	std::wstring _profileName;
	PrivateProfile _profile;
protected:
	PrivateProfileTest() {
		// Can use GetParam() here.
	}
	virtual ~PrivateProfileTest() {
		// Can use GetParam() here.
	}
	virtual void SetUp() override {
		// Can use GetParam() here.
		_profileName = generateProfileName();
		::DeleteFile(_profileName.c_str());
		_profile = std::move(PrivateProfile(_profileName));
	}
	virtual void TearDown() override {
		// Can use GetParam() here.
		::DeleteFile(_profileName.c_str());
	}
private:
	// ƒoƒbƒ`–¼‚ð¶¬‚·‚é
	std::wstring generateProfileName()
	{
		WCHAR fnTemplate[MAX_PATH] = { 0 };
		::wcscpy_s(fnTemplate, getEnvStr(L"TEMP").c_str());
		::wcscat_s(fnTemplate, L"\\prXXXXXX");
		::_wmktemp_s(fnTemplate);
		::wcscat_s(fnTemplate, L".ini");
		return std::wstring(fnTemplate);
	}
};
TEST_F(PrivateProfileTest, constructWithNoArg)
{
	_profile = std::move(PrivateProfile());
	ASSERT_FALSE(_profile.valid());
}

TEST_F(PrivateProfileTest, constructWithName)
{
	WCHAR name[] = L"test.ini";
	_profile = std::move(PrivateProfile(name));
	EXPECT_TRUE(_profile.valid());
	ASSERT_STREQ(name, _profile.GetProfileName().c_str());
}

TEST_F(PrivateProfileTest, constructWithMoveCtor)
{
	WCHAR name[] = L"test.ini";
	_profile = std::move(PrivateProfile(name));
	EXPECT_TRUE(_profile.valid());
	EXPECT_STREQ(name, _profile.GetProfileName().c_str());

	PrivateProfile movedProfile(std::move(_profile));
	EXPECT_TRUE(movedProfile.valid());
	ASSERT_STREQ(name, movedProfile.GetProfileName().c_str());

	ASSERT_FALSE(_profile.valid());
	ASSERT_STRNE(name, _profile.GetProfileName().c_str());
}


TEST_F(PrivateProfileTest, WriteAndRead) {
	// Can use GetParam() method here.

	// Gets information about the currently running test.
	// Do NOT delete the returned object - it's managed by the UnitTest class.
	const ::testing::TestInfo* const test_info =
		::testing::UnitTest::GetInstance()->current_test_info();

	const char* pchAppName = test_info->name();
	const char* pchKeyName = test_info->test_case_name();
	size_t cchAppName = strlen(pchAppName);
	size_t cchKeyName = strlen(pchKeyName);

	std::wstring appName(convertMbsToWString(pchAppName, cchAppName));
	std::wstring keyName(convertMbsToWString(pchKeyName, cchKeyName));

	std::wstring testValue(0x2000, L'w');
	_profile.writeString(appName.c_str(), keyName.c_str(), testValue.c_str());
	std::wstring val(_profile.getString(appName.c_str(), keyName.c_str()));
	ASSERT_EQ(testValue, val);
}


} //end of namespace test
