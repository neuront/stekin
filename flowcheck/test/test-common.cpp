#include <test/common.h>
#include <test/phony-errors.h>
#include <test/phony-warnings.h>
#include <proto/function.h>
#include <proto/func-inst-draft.h>
#include <instance/node-base.h>
#include <util/string.h>

#include "test-common.h"

using namespace test;

util::sref<proto::FuncInstDraft> const test::nul_draft(nullptr);
util::sref<proto::SymbolTable> const test::nul_st(nullptr);
misc::trace test::nultrace;

void test::instBlock(util::sref<proto::Block> block)
{
    block->inst(nul_draft, nultrace);
}

DataTree& DataTree::operator()(misc::position const& pos
                             , NodeType const& type
                             , std::string const& str)
{
    BaseType::operator()(type, FlowcheckData(pos), str);
    return *this;
}

DataTree& DataTree::operator()(misc::position const& pos, NodeType const& type)
{
    BaseType::operator()(type, FlowcheckData(pos));
    return *this;
}

DataTree& DataTree::operator()(misc::position const& pos, NodeType const& type, int list_size)
{
    BaseType::operator()(type, FlowcheckData(pos, list_size));
    return *this;
}

DataTree& DataTree::operator()(misc::position const& pos
                             , NodeType const& type
                             , std::string const& str
                             , int func_arg_size
                             , bool func_hint_ret_void)
{
    BaseType::operator()(type, FlowcheckData(pos, func_arg_size, func_hint_ret_void), str);
    return *this;
}

DataTree& DataTree::operator()(NodeType const& type)
{
    BaseType::operator()(type, FlowcheckData());
    return *this;
}

std::ostream& operator<<(std::ostream& os, FlowcheckData const& data)
{
    return -1 == data.func_arg_size
        ? (os << data.pos)
        : (os << data.pos << " arg size=" << data.func_arg_size
                          << " returning void=" << util::str(data.func_hint_ret_void));
}

NodeType const test::BOOLEAN("boolean");
NodeType const test::INTEGER("integer");
NodeType const test::FLOATING("floating");
NodeType const test::LIST("list");
NodeType const test::BINARY_OP("binary operation");
NodeType const test::PRE_UNARY_OP("prefix unary operation");
NodeType const test::CALL("call");
NodeType const test::FUNC_REFERENCE("func reference");
NodeType const test::FUNCTOR("functor");
NodeType const test::REFERENCE("var reference");
NodeType const test::LIST_ELEMENT("list element");
NodeType const test::LIST_INDEX("list index");

NodeType const test::LIST_PIPELINE("list pipeline");
NodeType const test::PIPE_MAP("pipe map");
NodeType const test::PIPE_FILTER("pipe filter");

NodeType const test::VAR_DEF("var definition");

NodeType const test::STATEMENT("statement");
NodeType const test::ARITHMETICS("arithmetics");
NodeType const test::RETURN("return");
NodeType const test::RETURN_NOTHING("return nothing");

NodeType const test::FUNC_DECL("func decl");
NodeType const test::PARAMETER("parameter");

NodeType const test::SCOPE_BEGIN("scope begin");
NodeType const test::SCOPE_END("scope end");

NodeType const test::BRANCH("branch");

void FlowcheckTest::SetUp()
{
    clearErr();
    clearWarn();
}

void FlowcheckTest::TearDown()
{
    DataTree::verify();
}
