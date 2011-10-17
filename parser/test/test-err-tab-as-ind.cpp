#include <gtest/gtest.h>

#include <test/phony-errors.h>
#include <test/common.h>

#include "../yy-misc.h"

using namespace test;

TEST(Syntax, ErrTabAsIndent)
{
    yyparse();
    ASSERT_TRUE(error::hasError());
    std::vector<TabAsIndRec> recs = getTabAsIndents();
    ASSERT_EQ(3, recs.size());

    EXPECT_EQ(misc::position(2), recs[0].pos);
    EXPECT_EQ(misc::position(4), recs[1].pos);
    EXPECT_EQ(misc::position(7), recs[2].pos);
}
