#include <gtest/gtest.h>

#include <test/phony-errors.h>

#include "../yy-misc.h"

TEST(Syntax, Empty)
{
    yyparse();
    ASSERT_FALSE(error::hasError());
}
