#include <algorithm>

#include <flowcheck/func-body-filter.h>
#include <flowcheck/global-filter.h>
#include <flowcheck/symbol-def-filter.h>
#include <flowcheck/accumulator.h>
#include <flowcheck/block.h>
#include <flowcheck/expr-nodes.h>
#include <flowcheck/list-pipe.h>
#include <flowcheck/stmt-nodes.h>
#include <flowcheck/function.h>
#include <proto/list-pipe.h>
#include <util/string.h>

#include "test-common.h"

using namespace test;
using namespace flchk;

namespace {

    util::sptr<proto::Statement> nulProtoStmt()
    {
        return util::sptr<proto::Statement>(nullptr);
    }

    util::sptr<proto::Expression const> nulProtoExpr()
    {
        return util::sptr<proto::Expression const>(nullptr);
    }

    util::sptr<flchk::Expression const> nulFlchkExpr()
    {
        return util::sptr<flchk::Expression const>(nullptr);
    }

    struct BranchConsequence
        : public Statement
    {
        BranchConsequence(misc::position const& pos, util::sptr<Expression const> p, Block c)
            : Statement(pos)
            , predicate(std::move(p))
            , consequence(std::move(c))
        {}

        util::sptr<proto::Statement> compile(util::sref<proto::Block>
                                           , util::sref<SymbolTable>) const
        {
            DataTree::actualOne()(pos, BRANCH_CONSQ_ONLY);
            predicate->compile(nulblock, nulSymbols());
            DataTree::actualOne()(CONSEQUENCE);
            consequence.compile(nulblock, nulSymbols());
            return nulProtoStmt();
        }

        util::sptr<Expression const> const predicate;
        Block const consequence;
    };

    struct BranchAlternative
        : public Statement
    {
        BranchAlternative(misc::position const& pos, util::sptr<Expression const> p, Block a)
            : Statement(pos)
            , predicate(std::move(p))
            , alternative(std::move(a))
        {}

        util::sptr<proto::Statement> compile(util::sref<proto::Block>
                                           , util::sref<SymbolTable>) const
        {
            DataTree::actualOne()(pos, BRANCH_ALTER_ONLY);
            predicate->compile(nulblock, nulSymbols());
            DataTree::actualOne()(ALTERNATIVE);
            alternative.compile(nulblock, nulSymbols());
            return nulProtoStmt();
        }

        util::sptr<Expression const> const predicate;
        Block const alternative;
    };

}

util::sref<proto::Function> Function::compile(util::sref<proto::Block>)
{
    DataTree::actualOne()(pos, FUNC_DEF, name);
    std::for_each(param_names.begin()
                , param_names.end()
                , [&](std::string const& param)
                  {
                      DataTree::actualOne()(pos, PARAMETER, param);
                  });
    _body->compile(nulblock);
    return util::sref<proto::Function>(nullptr);
}

void Block::addStmt(util::sptr<Statement const> stmt)
{
    _stmts.push_back(std::move(stmt));
}

util::sref<Function> Block::defFunc(misc::position const& pos
                                  , std::string const& name
                                  , std::vector<std::string> const& param_names
                                  , util::sptr<Filter> body)
{
    _funcs.push_back(util::mkptr(new Function(pos, name, param_names, std::move(body))));
    return *_funcs.back();
}

void Block::compile(util::sref<proto::Block>, util::sref<SymbolTable>) const 
{
    DataTree::actualOne()(BLOCK_BEGIN);
    std::for_each(_funcs.begin()
                , _funcs.end()
                , [&](util::sptr<Function> const& func)
                  {
                      func->compile(nulblock);
                  });

    std::for_each(_stmts.begin()
                , _stmts.end()
                , [&](util::sptr<Statement const> const& stmt)
                  {
                      stmt->compile(nulblock, nulSymbols());
                  });
    DataTree::actualOne()(BLOCK_END);
}

void Accumulator::addReturn(misc::position const& pos, util::sptr<Expression const> ret_val)
{
    _block.addStmt(util::mkptr(new Return(pos, std::move(ret_val))));
}

void Accumulator::addReturnNothing(misc::position const& pos)
{
    _block.addStmt(util::mkptr(new ReturnNothing(pos)));
}

void Accumulator::addArith(misc::position const& pos, util::sptr<Expression const> expr)
{
    _block.addStmt(util::mkptr(new Arithmetics(pos, std::move(expr))));
}

void Accumulator::addBranch(misc::position const& pos
                          , util::sptr<Expression const> predicate
                          , Accumulator consequence
                          , Accumulator alternative)
{
    _block.addStmt(util::mkptr(new Branch(pos
                                        , std::move(predicate)
                                        , std::move(consequence._block)
                                        , std::move(alternative._block))));
}

void Accumulator::addBranch(misc::position const& pos
                          , util::sptr<Expression const> predicate
                          , Accumulator consequence)
{
    _block.addStmt(util::mkptr(new BranchConsequence(pos
                                                   , std::move(predicate)
                                                   , std::move(consequence._block))));
}

void Accumulator::addBranchAlterOnly(misc::position const& pos
                                   , util::sptr<Expression const> predicate
                                   , Accumulator alternative)
{
    _block.addStmt(util::mkptr(new BranchAlternative(pos
                                                   , std::move(predicate)
                                                   , std::move(alternative._block))));
}

void Accumulator::defVar(misc::position const& pos
                       , std::string const& name
                       , util::sptr<Expression const> init)
{
    _block.addStmt(util::mkptr(new VarDef(pos, name, std::move(init))));
}

util::sref<Function> Accumulator::defFunc(misc::position const& pos
                                        , std::string const& name
                                        , std::vector<std::string> const& param_names
                                        , util::sptr<Filter> body)
{
    return _block.defFunc(pos, name, param_names, std::move(body));
}

void Accumulator::compileBlock(util::sref<proto::Block> block, util::sref<SymbolTable>) const
{
    _block.compile(block, nulSymbols());
}

void Filter::addReturn(misc::position const& pos, util::sptr<Expression const> ret_val)
{
    _accumulator.addReturn(pos, std::move(ret_val));
}

void Filter::addReturnNothing(misc::position const& pos)
{
    _accumulator.addReturnNothing(pos);
}

void Filter::addArith(misc::position const& pos, util::sptr<Expression const> expr)
{
    _accumulator.addArith(pos, std::move(expr));
}

void Filter::addBranch(misc::position const& pos
                     , util::sptr<Expression const> predicate
                     , util::sptr<Filter> consequence
                     , util::sptr<Filter> alternative)
{
    _accumulator.addBranch(pos
                         , std::move(predicate)
                         , std::move(consequence->_accumulator)
                         , std::move(alternative->_accumulator));
}

void Filter::addBranch(misc::position const& pos
                     , util::sptr<Expression const> predicate
                     , util::sptr<Filter> consequence)
{
    _accumulator.addBranch(pos, std::move(predicate), std::move(consequence->_accumulator));
}

void Filter::addBranchAlterOnly(misc::position const& pos
                              , util::sptr<Expression const> predicate
                              , util::sptr<Filter> alternative)
{
    _accumulator.addBranchAlterOnly(pos
                                  , std::move(predicate)
                                  , std::move(alternative->_accumulator));
}

void Filter::compile(util::sref<proto::Block> block)
{
    _accumulator.compileBlock(block, nulSymbols());
}

void FuncBodyFilter::defVar(misc::position const& pos
                          , std::string const& name
                          , util::sptr<Expression const> init)
{
    _accumulator.defVar(pos, name, std::move(init));
}

void FuncBodyFilter::defFunc(misc::position const& pos
                           , std::string const& name
                           , std::vector<std::string> const& param_names
                           , util::sptr<Filter> body)
{
    _accumulator.defFunc(pos, name, param_names, std::move(body));
}

util::sref<SymbolTable> FuncBodyFilter::getSymbols()
{
    return nulSymbols();
}

void SymbolDefFilter::defVar(misc::position const& pos
                           , std::string const& name
                           , util::sptr<Expression const> init)
{
    _accumulator.defVar(pos, name + VAR_DEF_FILTERED, std::move(init));
}

void SymbolDefFilter::defFunc(misc::position const& pos
                            , std::string const& name
                            , std::vector<std::string> const& param_names
                            , util::sptr<Filter> body)
{
    _accumulator.defFunc(pos, name + FUNC_DEF_FILTERED, param_names, std::move(body));
}

util::sref<SymbolTable> SymbolDefFilter::getSymbols()
{
    return nulSymbols();
}

GlobalFilter::GlobalFilter()
    : _writer_func(nulSymbols())
    , _selector_func(nulSymbols())
{}

util::sptr<Filter> WriterFunction::_mkBody(util::sref<SymbolTable> s)
{
    return util::mkptr(new SymbolDefFilter(s));
}

util::sptr<Filter> SelectorFunction::_mkBody(util::sref<SymbolTable> s)
{
    return util::mkptr(new SymbolDefFilter(s));
}

util::sptr<proto::Statement> Arithmetics::compile(util::sref<proto::Block>
                                                , util::sref<SymbolTable>) const 
{
    DataTree::actualOne()(pos, ARITHMETICS);
    expr->compile(nulblock, nulSymbols());
    return nulProtoStmt();
}

util::sptr<proto::Statement> Branch::compile(util::sref<proto::Block>
                                           , util::sref<SymbolTable>) const 
{
    DataTree::actualOne()(pos, BRANCH);
    predicate->compile(nulblock, nulSymbols());
    DataTree::actualOne()(CONSEQUENCE);
    consequence.compile(nulblock, nulSymbols());
    DataTree::actualOne()(ALTERNATIVE);
    alternative.compile(nulblock, nulSymbols());
    return nulProtoStmt();
}

util::sptr<proto::Statement> VarDef::compile(util::sref<proto::Block>
                                           , util::sref<SymbolTable>) const 
{
    DataTree::actualOne()(pos, VAR_DEF, name);
    init->compile(nulblock, nulSymbols());
    return nulProtoStmt();
}

util::sptr<proto::Statement> Return::compile(util::sref<proto::Block>
                                           , util::sref<SymbolTable>) const 
{
    DataTree::actualOne()(pos, RETURN);
    ret_val->compile(nulblock, nulSymbols());
    return nulProtoStmt();
}

util::sptr<proto::Statement> ReturnNothing::compile(util::sref<proto::Block>
                                                  , util::sref<SymbolTable>) const 
{
    DataTree::actualOne()(pos, RETURN_NOTHING);
    return nulProtoStmt();
}

util::sptr<proto::Expression const> PreUnaryOp::compile(util::sref<proto::Block>
                                                      , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, PRE_UNARY_OP, op_img);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> BinaryOp::compile(util::sref<proto::Block>
                                                    , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, BINARY_OP, op_img);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> Conjunction::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, BINARY_OP, "&&");
    return nulProtoExpr();
}

util::sptr<proto::Expression const> Disjunction::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, BINARY_OP, "||");
    return nulProtoExpr();
}

util::sptr<proto::Expression const> Negation::compile(util::sref<proto::Block>
                                                    , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, PRE_UNARY_OP, "!");
    return nulProtoExpr();
}

util::sptr<proto::Expression const> Reference::compile(util::sref<proto::Block>
                                                     , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, REFERENCE, name);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> BoolLiteral::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, BOOLEAN, util::str(value));
    return nulProtoExpr();
}

util::sptr<proto::Expression const> IntLiteral::compile(util::sref<proto::Block>
                                                      , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, INTEGER, util::str(value));
    return nulProtoExpr();
}

util::sptr<proto::Expression const> FloatLiteral::compile(util::sref<proto::Block>
                                                        , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, FLOATING, util::str(value));
    return nulProtoExpr();
}

static void compileList(std::vector<util::sptr<Expression const>> const& values)
{
    std::for_each(values.begin()
                , values.end()
                , [&](util::sptr<Expression const> const& v)
                  {
                      v->compile(nulblock, nulSymbols());
                  });
}

util::sptr<proto::Expression const> ListLiteral::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, LIST, value.size());
    compileList(value);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> ListElement::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, LIST_ELEMENT);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> ListIndex::compile(util::sref<proto::Block>
                                                     , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, LIST_INDEX);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> ListAppend::compile(util::sref<proto::Block>
                                                      , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, BINARY_OP, "++");
    lhs->compile(nulblock, nulSymbols());
    rhs->compile(nulblock, nulSymbols());
    return nulProtoExpr();
}

util::sptr<proto::Expression const> Call::compile(util::sref<proto::Block>
                                                , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, CALL, name, args.size());
    compileList(args);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> MemberCall::compile(util::sref<proto::Block>
                                                      , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, BINARY_OP, ".");
    object->compile(nulblock, nulSymbols());
    call->compile(nulblock, nulSymbols());
    return nulProtoExpr();
}

util::sptr<proto::Expression const> FuncReference::compile(util::sref<proto::Block>
                                                         , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, FUNC_REFERENCE, name, param_count);
    return nulProtoExpr();
}

util::sptr<proto::Expression const> ListPipeline::compile(util::sref<proto::Block>
                                                        , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(pos, LIST_PIPELINE, pipeline.size());
    list->compile(nulblock, nulSymbols());
    std::for_each(pipeline.begin()
                , pipeline.end()
                , [&](util::sptr<PipeBase const> const& pipe)
                  {
                      pipe->compile(nulblock, nulSymbols());
                  });
    return nulProtoExpr();
}

util::sptr<proto::PipeBase const> PipeMap::compile(util::sref<proto::Block>
                                                 , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(expr->pos, PIPE_MAP);
    expr->compile(nulblock, nulSymbols());
    return util::sptr<proto::PipeBase const>(nullptr);
}

util::sptr<proto::PipeBase const> PipeFilter::compile(util::sref<proto::Block>
                                                    , util::sref<SymbolTable>) const
{
    DataTree::actualOne()(expr->pos, PIPE_FILTER);
    expr->compile(nulblock, nulSymbols());
    return util::sptr<proto::PipeBase const>(nullptr);
}

bool Expression::isLiteral() const
{
    return false;
}

bool Expression::boolValue() const
{
    return false;
}

util::sptr<Expression const> Expression::operate(misc::position const&
                                               , std::string const&
                                               , mpz_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> Expression::operate(misc::position const&
                                               , std::string const&
                                               , mpf_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> Expression::operate(misc::position const&
                                               , std::string const&
                                               , bool) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> Expression::asRhs(misc::position const&
                                             , std::string const&
                                             , util::sptr<Expression const>) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> Expression::asRhs(misc::position const&, std::string const&) const
{
    return nulFlchkExpr();
}

bool PreUnaryOp::isLiteral() const
{
    return false;
}

bool PreUnaryOp::boolValue() const
{
    return false;
}

std::string PreUnaryOp::typeName() const
{
    return "";
}

util::sptr<Expression const> PreUnaryOp::fold() const
{
    return nulFlchkExpr();
}

bool BinaryOp::isLiteral() const
{
    return false;
}

bool BinaryOp::boolValue() const
{
    return false;
}

std::string BinaryOp::typeName() const
{
    return "";
}

util::sptr<Expression const> BinaryOp::fold() const
{
    return nulFlchkExpr();
}

bool Conjunction::isLiteral() const
{
    return false;
}

bool Conjunction::boolValue() const
{
    return false;
}

std::string Conjunction::typeName() const
{
    return "";
}

util::sptr<Expression const> Conjunction::fold() const
{
    return nulFlchkExpr();
}

bool Disjunction::isLiteral() const
{
    return false;
}

bool Disjunction::boolValue() const
{
    return false;
}

std::string Disjunction::typeName() const
{
    return "";
}

util::sptr<Expression const> Disjunction::fold() const
{
    return nulFlchkExpr();
}

bool Negation::isLiteral() const
{
    return false;
}

bool Negation::boolValue() const
{
    return false;
}

std::string Negation::typeName() const
{
    return "";
}

util::sptr<Expression const> Negation::fold() const
{
    return nulFlchkExpr();
}

std::string Reference::typeName() const
{
    return "";
}

util::sptr<Expression const> Reference::fold() const
{
    return nulFlchkExpr();
}

bool BoolLiteral::isLiteral() const
{
    return false;
}

bool BoolLiteral::boolValue() const
{
    return false;
}

std::string BoolLiteral::typeName() const
{
    return "";
}

util::sptr<Expression const> BoolLiteral::fold() const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> BoolLiteral::operate(misc::position const&
                                                , std::string const&
                                                , mpz_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> BoolLiteral::operate(misc::position const&
                                                , std::string const&
                                                , mpf_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> BoolLiteral::operate(misc::position const&
                                                , std::string const&
                                                , bool) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> BoolLiteral::asRhs(misc::position const&
                                              , std::string const&
                                              , util::sptr<Expression const>) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> BoolLiteral::asRhs(misc::position const&, std::string const&) const
{
    return nulFlchkExpr();
}

bool IntLiteral::isLiteral() const
{
    return false;
}

bool IntLiteral::boolValue() const
{
    return false;
}

std::string IntLiteral::typeName() const
{
    return "";
}

util::sptr<Expression const> IntLiteral::fold() const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> IntLiteral::operate(misc::position const&
                                               , std::string const&
                                               , mpz_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> IntLiteral::operate(misc::position const&
                                               , std::string const&
                                               , mpf_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> IntLiteral::operate(misc::position const&
                                               , std::string const&
                                               , bool) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> IntLiteral::asRhs(misc::position const&
                                             , std::string const&
                                             , util::sptr<Expression const>) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> IntLiteral::asRhs(misc::position const&, std::string const&) const
{
    return nulFlchkExpr();
}

bool FloatLiteral::isLiteral() const
{
    return false;
}

bool FloatLiteral::boolValue() const
{
    return false;
}

std::string FloatLiteral::typeName() const
{
    return "";
}

util::sptr<Expression const> FloatLiteral::fold() const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> FloatLiteral::operate(misc::position const&
                                                 , std::string const&
                                                 , mpz_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> FloatLiteral::operate(misc::position const&
                                                 , std::string const&
                                                 , mpf_class const&) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> FloatLiteral::operate(misc::position const&
                                                 , std::string const&
                                                 , bool) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> FloatLiteral::asRhs(misc::position const&
                                               , std::string const&
                                               , util::sptr<Expression const>) const
{
    return nulFlchkExpr();
}

util::sptr<Expression const> FloatLiteral::asRhs(misc::position const&, std::string const&) const
{
    return nulFlchkExpr();
}

std::string ListLiteral::typeName() const
{
    return "";
}

util::sptr<Expression const> ListLiteral::fold() const
{
    return nulFlchkExpr();
}

std::string ListElement::typeName() const
{
    return "";
}

util::sptr<Expression const> ListElement::fold() const
{
    return nulFlchkExpr();
}

std::string ListIndex::typeName() const
{
    return "";
}

util::sptr<Expression const> ListIndex::fold() const
{
    return nulFlchkExpr();
}

std::string ListAppend::typeName() const
{
    return "";
}

util::sptr<Expression const> ListAppend::fold() const
{
    return nulFlchkExpr();
}

std::string Call::typeName() const
{
    return "";
}

util::sptr<Expression const> Call::fold() const
{
    return nulFlchkExpr();
}

std::string MemberCall::typeName() const
{
    return "";
}

util::sptr<Expression const> MemberCall::fold() const
{
    return nulFlchkExpr();
}

std::string FuncReference::typeName() const
{
    return "";
}

util::sptr<Expression const> FuncReference::fold() const
{
    return nulFlchkExpr();
}

std::string ListPipeline::typeName() const
{
    return "";
}

util::sptr<Expression const> ListPipeline::fold() const
{
    return nulFlchkExpr();
}

util::sptr<PipeBase const> PipeMap::fold() const
{
    return util::sptr<PipeBase const>(nullptr);
}

util::sptr<PipeBase const> PipeFilter::fold() const
{
    return util::sptr<PipeBase const>(nullptr);
}
