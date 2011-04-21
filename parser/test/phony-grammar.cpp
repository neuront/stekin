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

static util::sptr<flchk::Expression const> nullptr()
{
    return std::move(util::sptr<flchk::Expression const>(0));
}

util::sptr<flchk::Expression const> pre_unary_op::compile() const
{
    data_tree::actual_one()(pos, PRE_UNARY_OP_BEGIN, op_img)(pos, OPERAND);
    rhs->compile();
    data_tree::actual_one()(pos, PRE_UNARY_OP_END);
    return std::move(nullptr());
}

util::sptr<flchk::Expression const> binary_op::compile() const
{
    data_tree::actual_one()(pos, BINARY_OP_BEGIN, op_img)(pos, OPERAND);
    lhs->compile();
    data_tree::actual_one()(pos, OPERAND);
    rhs->compile();
    data_tree::actual_one()(pos, BINARY_OP_END);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> conjunction::compile() const
{
    data_tree::actual_one()(pos, BINARY_OP_BEGIN, "&&")(pos, OPERAND);
    lhs->compile();
    data_tree::actual_one()(pos, OPERAND);
    rhs->compile();
    data_tree::actual_one()(pos, BINARY_OP_END);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> disjunction::compile() const
{
    data_tree::actual_one()(pos, BINARY_OP_BEGIN, "||")(pos, OPERAND);
    lhs->compile();
    data_tree::actual_one()(pos, OPERAND);
    rhs->compile();
    data_tree::actual_one()(pos, BINARY_OP_END);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> negation::compile() const
{
    data_tree::actual_one()(pos, PRE_UNARY_OP_BEGIN, "!")(pos, OPERAND);
    rhs->compile();
    data_tree::actual_one()(pos, PRE_UNARY_OP_END);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> reference::compile() const
{
    data_tree::actual_one()(pos, IDENTIFIER, name);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> bool_literal::compile() const
{
    data_tree::actual_one()(pos, BOOLEAN, util::str(value));
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> IntLiteral::compile() const
{
    data_tree::actual_one()(pos, INTEGER, value);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> float_literal::compile() const
{
    data_tree::actual_one()(pos, FLOATING, value);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> call::compile() const
{
    data_tree::actual_one()(pos, FUNC_CALL_BEGIN, name);
    std::for_each(args.begin()
                , args.end()
                , [&](util::sptr<Expression const> const& expr)
                  {
                      data_tree::actual_one()(pos, ARGUMENT);
                      expr->compile();
                  });
    data_tree::actual_one()(pos, FUNC_CALL_END);
    return std::move(nullptr());;
}

util::sptr<flchk::Expression const> func_reference::compile() const
{
    data_tree::actual_one()(pos, IDENTIFIER, name + '@' + util::str(param_count));
    return std::move(nullptr());;
}

void clause_builder::add_arith(int indent_level, util::sptr<Expression const> arith)
{
    data_tree::actual_one()(arith->pos, indent_level, ARITHMETICS);
    util::sptr<Expression const>(std::move(arith))->compile();
}

void clause_builder::add_var_def(int indent_level, std::string const& name, util::sptr<Expression const> init)
{
    data_tree::actual_one()(init->pos, indent_level, VAR_DEF, name);
    util::sptr<Expression const>(std::move(init))->compile();
}

void clause_builder::add_return(int indent_level, util::sptr<Expression const> ret_val)
{
    data_tree::actual_one()(ret_val->pos, indent_level, RETURN);
    util::sptr<Expression const>(std::move(ret_val))->compile();
}

void clause_builder::add_return_nothing(int indent_level, misc::pos_type const& pos)
{
    data_tree::actual_one()(pos, indent_level, RETURN, "");
}

void clause_builder::add_function(int indent_level
                                , misc::pos_type const& pos
                                , std::string const& name
                                , std::vector<std::string> const& params)
{
    data_tree::actual_one()(pos, indent_level, FUNC_DEF_HEAD_BEGIN, name);
    std::for_each(params.begin()
                , params.end()
                , [&](std::string const& param)
                  {
                      data_tree::actual_one()(pos, indent_level, IDENTIFIER, param);
                  });
    data_tree::actual_one()(pos, indent_level, FUNC_DEF_HEAD_END);
}

void clause_builder::add_if(int indent_level, util::sptr<Expression const> condition)
{
    misc::pos_type pos(condition->pos);
    data_tree::actual_one()(pos, indent_level, BRANCH_IF)(pos, indent_level, CONDITION_BEGIN);
    util::sptr<Expression const>(std::move(condition))->compile();
    data_tree::actual_one()(pos, indent_level, CONDITION_END);
}

void clause_builder::add_ifnot(int indent_level, util::sptr<Expression const> condition)
{
    misc::pos_type pos(condition->pos);
    data_tree::actual_one()(pos, indent_level, BRANCH_IFNOT)(pos, indent_level, CONDITION_BEGIN);
    util::sptr<Expression const>(std::move(condition))->compile();
    data_tree::actual_one()(pos, indent_level, CONDITION_END);
}

void clause_builder::add_else(int indent_level, misc::pos_type const& pos)
{
    data_tree::actual_one()(pos, indent_level, BRANCH_ELSE);
}

flchk::block clause_builder::build_and_clear()
{
    return std::move(flchk::block());
}

acceptor_stack::acceptor_stack()
    : _packer(NULL)
{}
