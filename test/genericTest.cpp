#include "StdAfx.h"
#include "generic.h"

#include "resource.h"


namespace test {


class EnvStrTest : public ::testing::TestWithParam<LPCWSTR> {
protected:
	EnvStrTest() {
		// Can use GetParam() here.
	}
	virtual ~EnvStrTest() {
		// Can use GetParam() here.
	}
	virtual void SetUp() {
		// Can use GetParam() here.
	}
	virtual void TearDown() {
		// Can use GetParam() here.
	}
};
TEST_P(EnvStrTest, GetAndSet) {
	// Can use GetParam() method here.
	auto pszVarName = GetParam();
	auto envStr = getEnvStr(pszVarName);
	ASSERT_FALSE(envStr.empty());
	setEnvStr(pszVarName, L"");
	EXPECT_TRUE(getEnvStr(pszVarName).empty());
	setEnvStr(pszVarName, envStr.c_str());
	ASSERT_EQ(envStr, getEnvStr(pszVarName));
}
INSTANTIATE_TEST_CASE_P(EnvStrTest, EnvStrTest, ::testing::Values(L"ProgramFiles", L"ProgramW6432", L"Path"));


TEST(LoadStringTest, loadString)
{
	std::wstring text(loadString(IDS_BAD_ANSWER));
	ASSERT_FALSE(text.empty());
}

TEST(LoadStringTest, loadStringFail)
{
	ASSERT_THROW({ loadString((WORD)0xFFFFu); }, std::runtime_error);
}

} //end of namespace test
