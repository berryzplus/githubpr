#include "StdAfx.h"
#include "BatchCommand.h"

namespace test {


TEST(BatchCommandTest, constructWithNoArg)
{
	BatchCommand cmd;
	EXPECT_TRUE(cmd.GetName().empty());
	ASSERT_TRUE(cmd.GetContent().empty());
}

TEST(BatchCommandTest, constructWithCmdEcho)
{
	const WCHAR content[] = L"@echo test";
	BatchCommand cmd(content);
	EXPECT_FALSE(cmd.GetName().empty());
	EXPECT_FALSE(cmd.GetContent().empty());
	ASSERT_STREQ(content, cmd.GetContent().c_str());
}

TEST(BatchCommandTest, executeBatchWithoutContent)
{
	BatchCommand cmd;
	EXPECT_TRUE(cmd.GetContent().empty());

	ASSERT_THROW(cmd.invokeAndGetLines(), std::runtime_error);
}

TEST(BatchCommandTest, executeBatchWithCmdEcho)
{
	const WCHAR content[] = L"@echo test";
	const WCHAR testEcho[] = L"test";
	BatchCommand cmd(content);
	EXPECT_FALSE(cmd.GetName().empty());
	EXPECT_FALSE(cmd.GetContent().empty());

	auto retList = cmd.invokeAndGetLines();
	EXPECT_EQ(1, retList.size());
	ASSERT_STREQ(testEcho, retList.front().c_str());
}

} //end of namespace test
