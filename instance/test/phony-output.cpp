#include "test-common.h"
#include "../../output/func-writer.h"
#include "../../output/statement-writer.h"
#include "../../output/name-mangler.h"
#include "../../util/string.h"

using namespace test;
using namespace output;

void output::writeFuncDecl(std::string const& return_type_name
                         , util::serial_num
                         , std::list<StackVarRec> const& var_recs
                         , int level
                         , int stack_size)
{
    DataTree::actualOne()(FUNC_DECL_BEGIN, return_type_name, level, stack_size);
    std::for_each(var_recs.begin()
                , var_recs.end()
                , [&](StackVarRec const& var)
                  {
                      DataTree::actualOne()(PARAMETER, var.type, var.level, var.offset);
                  });
    DataTree::actualOne()(FUNC_DECL_END);
}

void output::writeFuncImpl(std::string const& return_type_name, util::serial_num)
{
    DataTree::actualOne()(FUNC_DEF, return_type_name);
}

void output::writeCallBegin(util::serial_num) {}
void output::writeArgSeparator() {}
void output::writeCallEnd() {}
void output::kwReturn() {}
void output::returnNothing() {}
void output::refThisLevel(int, std::string const&) {}

void output::refLevel(int offset, int level, std::string const& type_name)
{
    DataTree::actualOne()(REFERENCE, type_name, level, offset);
}

void output::signAssign() {}

void output::writeInt(platform::int_type value)
{
    DataTree::actualOne()(INTEGER, util::str(value));
}

void output::writeFloat(platform::float_type value)
{
    DataTree::actualOne()(FLOAT, util::str(value));
}

void output::writeBool(bool value)
{
    DataTree::actualOne()(BOOLEAN, util::str(value));
}

void output::writeOperator(std::string const&) {}
void output::beginExpr() {}
void output::endExpr() {}
void output::branchIf() {}
void output::branchElse() {}

void output::blockBegin()
{
    DataTree::actualOne()(BLOCK_BEGIN);
}

void output::blockEnd()
{
    DataTree::actualOne()(BLOCK_END);
}

void output::endOfStatement() {}
void output::writeFuncReference(int) {}
void output::funcReferenceNextVariable(int, StackVarRec const&) {}
void output::beginWriteStmt() {}
void output::endWriteStmt() {}

std::string output::formFuncReferenceType(int size)
{
    return util::str(size);
}

std::string output::formType(std::string const& name)
{
    return name;
}
