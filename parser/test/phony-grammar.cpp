#include <algorithm>

#include "test-common.h"
#include "../../test/data-trees.h"
#include "../../grammar/expr-nodes.h"
#include "../../grammar/clause-builder.h"
#include "../../flowcheck/node-base.h"
#include "../../flowcheck/block.h"
#include "../../flowcheck/function.h"
#include "../../proto/node-base.h"
#include "../../util/string.h"

using namespace grammar;
using namespace test;

static util::sptr<flchk::Expression const> nulptr(NULL);

util::sptr<flchk::Expression const> PreUnaryOp::compile() const
{
    DataTree::actualOne()(pos, PRE_UNARY_OP_BEGIN, op_img)(pos, OPERAND);
    rhs->compile();
    DataTree::actualOne()(pos, PRE_UNARY_OP_END);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> BinaryOp::compile() const
{
    DataTree::actualOne()(pos, BINARY_OP_BEGIN, op_img)(pos, OPERAND);
    lhs->compile();
    DataTree::actualOne()(pos, OPERAND);
    rhs->compile();
    DataTree::actualOne()(pos, BINARY_OP_END);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> Conjunction::compile() const
{
    DataTree::actualOne()(pos, BINARY_OP_BEGIN, "&&")(pos, OPERAND);
    lhs->compile();
    DataTree::actualOne()(pos, OPERAND);
    rhs->compile();
    DataTree::actualOne()(pos, BINARY_OP_END);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> Disjunction::compile() const
{
    DataTree::actualOne()(pos, BINARY_OP_BEGIN, "||")(pos, OPERAND);
    lhs->compile();
    DataTree::actualOne()(pos, OPERAND);
    rhs->compile();
    DataTree::actualOne()(pos, BINARY_OP_END);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> Negation::compile() const
{
    DataTree::actualOne()(pos, PRE_UNARY_OP_BEGIN, "!")(pos, OPERAND);
    rhs->compile();
    DataTree::actualOne()(pos, PRE_UNARY_OP_END);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> Reference::compile() const
{
    DataTree::actualOne()(pos, IDENTIFIER, name);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> BoolLiteral::compile() const
{
    DataTree::actualOne()(pos, BOOLEAN, util::str(value));
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> IntLiteral::compile() const
{
    DataTree::actualOne()(pos, INTEGER, value);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> FloatLiteral::compile() const
{
    DataTree::actualOne()(pos, FLOATING, value);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> Call::compile() const
{
    DataTree::actualOne()(pos, FUNC_CALL_BEGIN, name);
    std::for_each(args.begin()
                , args.end()
                , [&](util::sptr<Expression const> const& expr)
                  {
                      DataTree::actualOne()(pos, ARGUMENT);
                      expr->compile();
                  });
    DataTree::actualOne()(pos, FUNC_CALL_END);
    return std::move(nulptr);
}

util::sptr<flchk::Expression const> FuncReference::compile() const
{
    DataTree::actualOne()(pos, IDENTIFIER, name + '@' + util::str(param_count));
    return std::move(nulptr);
}

void ClauseBuilder::addArith(int indent_level, util::sptr<Expression const> arith)
{
    DataTree::actualOne()(arith->pos, indent_level, ARITHMETICS);
    arith->compile();
}

void ClauseBuilder::addVarDef(int indent_level
                            , std::string const& name
                            , util::sptr<Expression const> init)
{
    DataTree::actualOne()(init->pos, indent_level, VAR_DEF, name);
    init->compile();
}

void ClauseBuilder::addReturn(int indent_level, util::sptr<Expression const> ret_val)
{
    DataTree::actualOne()(ret_val->pos, indent_level, RETURN);
    ret_val->compile();
}

void ClauseBuilder::addReturnNothing(int indent_level, misc::position const& pos)
{
    DataTree::actualOne()(pos, indent_level, RETURN, "");
}

void ClauseBuilder::addFunction(int indent_level
                              , misc::position const& pos
                              , std::string const& name
                              , std::vector<std::string> const& params)
{
    DataTree::actualOne()(pos, indent_level, FUNC_DEF_HEAD_BEGIN, name);
    std::for_each(params.begin()
                , params.end()
                , [&](std::string const& param)
                  {
                      DataTree::actualOne()(pos, indent_level, IDENTIFIER, param);
                  });
    DataTree::actualOne()(pos, indent_level, FUNC_DEF_HEAD_END);
}

void ClauseBuilder::addIf(int indent_level, util::sptr<Expression const> condition)
{
    misc::position pos(condition->pos);
    DataTree::actualOne()(pos, indent_level, BRANCH_IF)(pos, indent_level, CONDITION_BEGIN);
    condition->compile();
    DataTree::actualOne()(pos, indent_level, CONDITION_END);
}

void ClauseBuilder::addIfnot(int indent_level, util::sptr<Expression const> condition)
{
    misc::position pos(condition->pos);
    DataTree::actualOne()(pos, indent_level, BRANCH_IFNOT)(pos, indent_level, CONDITION_BEGIN);
    condition->compile();
    DataTree::actualOne()(pos, indent_level, CONDITION_END);
}

void ClauseBuilder::addElse(int indent_level, misc::position const& pos)
{
    DataTree::actualOne()(pos, indent_level, BRANCH_ELSE);
}

flchk::Block ClauseBuilder::buildAndClear()
{
    return std::move(flchk::Block());
}

AcceptorStack::AcceptorStack()
    : _packer(NULL)
{}
