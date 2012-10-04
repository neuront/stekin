#include <algorithm>
#include <map>

#include <proto/variable.h>
#include <proto/expr-nodes.h>
#include <proto/func-inst-draft.h>
#include <util/string.h>
#include <report/errors.h>

#include "expr-nodes.h"
#include "function.h"
#include "symbol-table.h"
#include "filter.h"

using namespace flchk;

namespace {

    static util::sptr<Expression const> makeFakeExpr(misc::position const& pos)
    {
        return util::mkptr(new BoolLiteral(pos, false));
    }

    struct LiteralBinaryOp {
        virtual util::sptr<Expression const> operate(misc::position const& pos
                                                   , mpz_class const& lhs
                                                   , mpz_class const& rhs) const = 0;
        virtual util::sptr<Expression const> operate(misc::position const& pos
                                                   , mpz_class const& lhs
                                                   , mpf_class const& rhs) const = 0;
        virtual util::sptr<Expression const> operate(misc::position const& pos
                                                   , mpf_class const& lhs
                                                   , mpz_class const& rhs) const = 0;
        virtual util::sptr<Expression const> operate(misc::position const& pos
                                                   , mpf_class const& lhs
                                                   , mpf_class const& rhs) const = 0;
    };

    template <typename _OpFunc>
    struct BinaryOpImplement
        : LiteralBinaryOp
    {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operateImpl(misc::position const& pos
                                               , _Lhs const& lhs
                                               , _Rhs const& rhs) const
        {
            return _OpFunc()(pos, lhs, rhs);
        }

        util::sptr<Expression const> operate(misc::position const& pos
                                           , mpz_class const& lhs
                                           , mpz_class const& rhs) const
        {
            return operateImpl(pos, lhs, rhs);
        }

        util::sptr<Expression const> operate(misc::position const& pos
                                           , mpz_class const& lhs
                                           , mpf_class const& rhs) const
        {
            return operateImpl(pos, lhs, rhs);
        }

        util::sptr<Expression const> operate(misc::position const& pos
                                           , mpf_class const& lhs
                                           , mpz_class const& rhs) const
        {
            return operateImpl(pos, lhs, rhs);
        }

        util::sptr<Expression const> operate(misc::position const& pos
                                           , mpf_class const& lhs
                                           , mpf_class const& rhs) const
        {
            return operateImpl(pos, lhs, rhs);
        }
    };

    struct LTBinary {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new BoolLiteral(pos, lhs < rhs));
        }
    };
    BinaryOpImplement<LTBinary> lt_binary_obj;

    struct LEBinary {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new BoolLiteral(pos, lhs <= rhs));
        }
    };
    BinaryOpImplement<LEBinary> le_binary_obj;

    struct GEBinary {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new BoolLiteral(pos, lhs >= rhs));
        }
    };
    BinaryOpImplement<GEBinary> ge_binary_obj;

    struct GTBinary {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new BoolLiteral(pos, lhs > rhs));
        }
    };
    BinaryOpImplement<GTBinary> gt_binary_obj;

    struct EQBinary {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new BoolLiteral(pos, lhs == rhs));
        }
    };
    BinaryOpImplement<EQBinary> eq_binary_obj;

    struct NEBinary {
        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new BoolLiteral(pos, lhs != rhs));
        }
    };
    BinaryOpImplement<NEBinary> ne_binary_obj;

    struct AddBinary {
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , mpz_class const& lhs
                                              , mpz_class const& rhs) const
        {
            return util::mkptr(new IntLiteral(pos, lhs + rhs));
        }

        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new FloatLiteral(pos, lhs + rhs));
        }
    };
    BinaryOpImplement<AddBinary> add_binary_obj;

    struct SubBinary {
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , mpz_class const& lhs
                                              , mpz_class const& rhs) const
        {
            return util::mkptr(new IntLiteral(pos, lhs - rhs));
        }

        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new FloatLiteral(pos, lhs - rhs));
        }
    };
    BinaryOpImplement<SubBinary> sub_binary_obj;

    struct MulBinary {
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , mpz_class const& lhs
                                              , mpz_class const& rhs) const
        {
            return util::mkptr(new IntLiteral(pos, lhs * rhs));
        }

        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new FloatLiteral(pos, lhs * rhs));
        }
    };
    BinaryOpImplement<MulBinary> mul_binary_obj;

    struct DivBinary {
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , mpz_class const& lhs
                                              , mpz_class const& rhs) const
        {
            if (0 == rhs) {
                error::binaryOpNotAvai(pos, "/", "int", "integer literal(0)");
                return makeFakeExpr(pos);
            }
            return util::mkptr(new IntLiteral(pos, lhs / rhs));
        }

        util::sptr<Expression const> operator()(misc::position const& pos
                                              , mpf_class const& lhs
                                              , mpz_class const& rhs) const
        {
            if (0 == rhs) {
                error::binaryOpNotAvai(pos, "/", "int", "integer literal(0)");
                return makeFakeExpr(pos);
            }
            return util::mkptr(new FloatLiteral(pos, lhs / rhs));
        }

        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const& lhs
                                              , _Rhs const& rhs) const
        {
            return util::mkptr(new FloatLiteral(pos, lhs / rhs));
        }
    };
    BinaryOpImplement<DivBinary> div_binary_obj;

    struct ModBinary {
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , mpz_class const& lhs
                                              , mpz_class const& rhs) const
        {
            if (0 == rhs) {
                error::binaryOpNotAvai(pos, "%", "int", "integer literal(0)");
                return makeFakeExpr(pos);
            }
            return util::mkptr(new IntLiteral(pos, lhs % rhs));
        }

        template <typename _Lhs, typename _Rhs>
        util::sptr<Expression const> operator()(misc::position const& pos
                                              , _Lhs const&
                                              , _Rhs const&) const
        {
            error::binaryOpNotAvai(pos, "%", "int or float", "int or float");
            return makeFakeExpr(pos);
        }
    };
    BinaryOpImplement<ModBinary> mod_binary_obj;

    std::map<std::string, util::sref<LiteralBinaryOp const>> makeCmpOpMap()
    {
        std::map<std::string, util::sref<LiteralBinaryOp const>> map;
        map.insert(std::make_pair("<", util::mkref(lt_binary_obj)));
        map.insert(std::make_pair("<=", util::mkref(le_binary_obj)));
        map.insert(std::make_pair(">=", util::mkref(ge_binary_obj)));
        map.insert(std::make_pair(">", util::mkref(gt_binary_obj)));
        map.insert(std::make_pair("=", util::mkref(eq_binary_obj)));
        map.insert(std::make_pair("!=", util::mkref(ne_binary_obj)));
        return map;
    }

    std::map<std::string, util::sref<LiteralBinaryOp const>> makeOpMap()
    {
        std::map<std::string, util::sref<LiteralBinaryOp const>> map(makeCmpOpMap());
        map.insert(std::make_pair("+", util::mkref(add_binary_obj)));
        map.insert(std::make_pair("-", util::mkref(sub_binary_obj)));
        map.insert(std::make_pair("*", util::mkref(mul_binary_obj)));
        map.insert(std::make_pair("/", util::mkref(div_binary_obj)));
        map.insert(std::make_pair("%", util::mkref(mod_binary_obj)));
        return map;
    }

    std::map<std::string, util::sref<LiteralBinaryOp const>> const COMPARE_OPS(makeCmpOpMap());
    std::map<std::string, util::sref<LiteralBinaryOp const>> const ALL_OPS(makeOpMap());

}

util::sptr<proto::Expression const> PreUnaryOp::compile(util::sref<proto::Block> block
                                                      , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::PreUnaryOp(pos, op_img, rhs->compile(block, st)));
}

bool PreUnaryOp::isLiteral() const
{
    return rhs->isLiteral();
}

bool PreUnaryOp::boolValue() const
{
    error::condNotBool(pos, typeName());
    return false;
}

std::string PreUnaryOp::typeName() const
{
    return '(' + op_img + rhs->typeName() + ')';
}

util::sptr<Expression const> PreUnaryOp::fold() const
{
    return rhs->fold()->asRhs(pos, op_img);
}

util::sptr<proto::Expression const> BinaryOp::compile(util::sref<proto::Block> block
                                                    , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::BinaryOp(pos
                                         , lhs->compile(block, st)
                                         , op_img
                                         , rhs->compile(block, st)));
}

bool BinaryOp::isLiteral() const
{
    return lhs->isLiteral() && rhs->isLiteral();
}

bool BinaryOp::boolValue() const
{
    if (COMPARE_OPS.end() == COMPARE_OPS.find(op_img)) {
        error::condNotBool(pos, typeName());
        return false;
    }
    return fold()->boolValue();
}

std::string BinaryOp::typeName() const
{
    return '(' + lhs->typeName() + op_img + rhs->typeName() + ')';
}

util::sptr<Expression const> BinaryOp::fold() const
{
    return rhs->fold()->asRhs(pos, op_img, lhs->fold());
}

util::sptr<proto::Expression const> Conjunction::compile(util::sref<proto::Block> block
                                                       , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::Conjunction(pos
                                            , lhs->compile(block, st)
                                            , rhs->compile(block, st)));
}

bool Conjunction::isLiteral() const
{
    return lhs->isLiteral() && rhs->isLiteral();
}

bool Conjunction::boolValue() const
{
    return lhs->boolValue() && rhs->boolValue();
}

std::string Conjunction::typeName() const
{
    return '(' + lhs->typeName() + "&&" + rhs->typeName() + ')';
}

util::sptr<Expression const> Conjunction::fold() const
{
    if (isLiteral()) {
        return util::mkptr(new BoolLiteral(pos, lhs->boolValue() && rhs->boolValue()));
    }
    return util::mkptr(new Conjunction(pos, lhs->fold(), rhs->fold()));
}

util::sptr<proto::Expression const> Disjunction::compile(util::sref<proto::Block> block
                                                       , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::Disjunction(pos
                                            , lhs->compile(block, st)
                                            , rhs->compile(block, st)));
}

bool Disjunction::isLiteral() const
{
    return lhs->isLiteral() && rhs->isLiteral();
}

bool Disjunction::boolValue() const
{
    return lhs->boolValue() || rhs->boolValue();
}

std::string Disjunction::typeName() const
{
    return '(' + lhs->typeName() + "||" + rhs->typeName() + ')';
}

util::sptr<Expression const> Disjunction::fold() const
{
    if (isLiteral()) {
        return util::mkptr(new BoolLiteral(pos, lhs->boolValue() || rhs->boolValue()));
    }
    return util::mkptr(new Disjunction(pos, lhs->fold(), rhs->fold()));
}

util::sptr<proto::Expression const> Negation::compile(util::sref<proto::Block> block
                                                    , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::Negation(pos, rhs->compile(block, st)));
}

bool Negation::isLiteral() const
{
    return rhs->isLiteral();
}

bool Negation::boolValue() const
{
    return !rhs->boolValue();
}

std::string Negation::typeName() const
{
    return "(!" + rhs->typeName() + ')';
}

util::sptr<Expression const> Negation::fold() const
{
    if (isLiteral()) {
        return util::mkptr(new BoolLiteral(pos, !rhs->boolValue()));
    }
    return util::mkptr(new Negation(pos, rhs->fold()));
}

util::sptr<proto::Expression const> Reference::compile(util::sref<proto::Block> block
                                                     , util::sref<SymbolTable> st) const
{
    return st->compileRef(pos, name, block);
}

std::string Reference::typeName() const
{
    return "(reference(" + name + "))";
}

util::sptr<Expression const> Reference::fold() const
{
    return util::mkptr(new Reference(pos, name));
}

util::sptr<proto::Expression const> BoolLiteral::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    return util::mkptr(new proto::BoolLiteral(pos, value));
}

bool BoolLiteral::isLiteral() const
{
    return true;
}

bool BoolLiteral::boolValue() const
{
    return value;
}

std::string BoolLiteral::typeName() const
{
    return "(bool(" + util::str(value) + "))";
}

util::sptr<Expression const> BoolLiteral::fold() const
{
    return util::mkptr(new BoolLiteral(pos, value));
}

util::sptr<Expression const> BoolLiteral::operate(misc::position const& op_pos
                                                , std::string const& op_img
                                                , mpz_class const&) const
{
    error::binaryOpNotAvai(op_pos, op_img, "bool", "int");
    return makeFakeExpr(op_pos);
}

util::sptr<Expression const> BoolLiteral::operate(misc::position const& op_pos
                                                , std::string const& op_img
                                                , mpf_class const&) const
{
    error::binaryOpNotAvai(op_pos, op_img, "bool", "float");
    return makeFakeExpr(op_pos);
}

util::sptr<Expression const> BoolLiteral::operate(misc::position const& op_pos
                                                , std::string const& op_img
                                                , bool rhs) const
{
    if ("=" == op_img) {
        return util::mkptr(new BoolLiteral(op_pos, value == rhs));
    }
    return util::mkptr(new BoolLiteral(op_pos, value != rhs));
}

util::sptr<Expression const> BoolLiteral::asRhs(misc::position const& op_pos
                                              , std::string const& op_img
                                              , util::sptr<Expression const> lhs) const
{
    return lhs->operate(op_pos, op_img, value);
}

util::sptr<Expression const> BoolLiteral::asRhs(misc::position const& op_pos
                                              , std::string const& op_img) const
{
    error::preUnaryOpNotAvai(op_pos, op_img, "bool");
    return makeFakeExpr(op_pos);
}

util::sptr<proto::Expression const> IntLiteral::compile(util::sref<proto::Block>
                                                      , util::sref<SymbolTable>) const
{
    return util::mkptr(new proto::IntLiteral(pos, value));
}

bool IntLiteral::isLiteral() const
{
    return true;
}

bool IntLiteral::boolValue() const
{
    error::condNotBool(pos, typeName());
    return false;
}

std::string IntLiteral::typeName() const
{
    return "(int(" + util::str(value) + "))";
}

util::sptr<Expression const> IntLiteral::fold() const
{
    return util::mkptr(new IntLiteral(pos, value));
}

util::sptr<Expression const> IntLiteral::operate(misc::position const& op_pos
                                               , std::string const& op_img
                                               , mpz_class const& rhs) const
{
    return ALL_OPS.find(op_img)->second->operate(op_pos, value, rhs);
}

util::sptr<Expression const> IntLiteral::operate(misc::position const& op_pos
                                               , std::string const& op_img
                                               , mpf_class const& rhs) const
{
    return ALL_OPS.find(op_img)->second->operate(op_pos, value, rhs);
}

util::sptr<Expression const> IntLiteral::operate(misc::position const& op_pos
                                               , std::string const& op_img
                                               , bool) const
{
    error::binaryOpNotAvai(op_pos, op_img, "int", "bool");
    return makeFakeExpr(op_pos);
}

util::sptr<Expression const> IntLiteral::asRhs(misc::position const& op_pos
                                             , std::string const& op_img
                                             , util::sptr<Expression const> lhs) const
{
    return lhs->operate(op_pos, op_img, value);
}

util::sptr<Expression const> IntLiteral::asRhs(misc::position const& op_pos
                                             , std::string const& op_img) const
{
    if ("-" == op_img) {
        return util::mkptr(new IntLiteral(op_pos, -value));
    }
    return util::mkptr(new IntLiteral(op_pos, value));
}

util::sptr<proto::Expression const> FloatLiteral::compile(util::sref<proto::Block>
                                                        , util::sref<SymbolTable>) const
{
    return util::mkptr(new proto::FloatLiteral(pos, value));
}

bool FloatLiteral::isLiteral() const
{
    return true;
}

bool FloatLiteral::boolValue() const
{
    error::condNotBool(pos, typeName());
    return false;
}

std::string FloatLiteral::typeName() const
{
    return "(float(" + util::str(value) + "))";
}

util::sptr<Expression const> FloatLiteral::fold() const
{
    return util::mkptr(new FloatLiteral(pos, value));
}

util::sptr<Expression const> FloatLiteral::operate(misc::position const& op_pos
                                                 , std::string const& op_img
                                                 , mpz_class const& rhs) const
{
    return ALL_OPS.find(op_img)->second->operate(op_pos, value, rhs);
}

util::sptr<Expression const> FloatLiteral::operate(misc::position const& op_pos
                                                 , std::string const& op_img
                                                 , mpf_class const& rhs) const
{
    return ALL_OPS.find(op_img)->second->operate(op_pos, value, rhs);
}

util::sptr<Expression const> FloatLiteral::operate(misc::position const& op_pos
                                                 , std::string const& op_img
                                                 , bool) const
{
    error::binaryOpNotAvai(op_pos, op_img, "float", "bool");
    return makeFakeExpr(op_pos);
}

util::sptr<Expression const> FloatLiteral::asRhs(misc::position const& op_pos
                                               , std::string const& op_img
                                               , util::sptr<Expression const> lhs) const
{
    return lhs->operate(op_pos, op_img, value);
}

util::sptr<Expression const> FloatLiteral::asRhs(misc::position const& op_pos
                                               , std::string const& op_img) const
{
    if ("-" == op_img) {
        return util::mkptr(new FloatLiteral(op_pos, -value));
    }
    return util::mkptr(new FloatLiteral(op_pos, value));
}

util::sptr<proto::Expression const> StringLiteral::compile(util::sref<proto::Block>
                                                         , util::sref<SymbolTable>) const
{
    return util::mkptr(new proto::StringLiteral(pos, value));
}

std::string StringLiteral::typeName() const
{
    return "(string(" + value + "))";
}

util::sptr<Expression const> StringLiteral::fold() const
{
    return util::mkptr(new StringLiteral(pos, value));
}

static std::vector<util::sptr<proto::Expression const>> compileList(
                        std::vector<util::sptr<Expression const>> const& list
                      , util::sref<proto::Block> block
                      , util::sref<SymbolTable> st)
{
    std::vector<util::sptr<proto::Expression const>> compiled_list;
    std::for_each(list.begin()
                , list.end()
                , [&](util::sptr<Expression const> const& value)
                  {
                      compiled_list.push_back(value->compile(block, st));
                  });
    return std::move(compiled_list);
}

util::sptr<proto::Expression const> ListLiteral::compile(util::sref<proto::Block> block
                                                       , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::ListLiteral(pos, compileList(value, block, st)));
}

std::string ListLiteral::typeName() const
{
    return "list";
}

static std::vector<util::sptr<Expression const>> foldList(
                        std::vector<util::sptr<Expression const>> const& list)
{
    std::vector<util::sptr<Expression const>> folded_list;
    std::for_each(list.begin()
                , list.end()
                , [&](util::sptr<Expression const> const& value)
                  {
                      folded_list.push_back(value->fold());
                  });
    return std::move(folded_list);
}

util::sptr<Expression const> ListLiteral::fold() const
{
    return util::mkptr(new ListLiteral(pos, foldList(value)));
}

util::sptr<proto::Expression const> ListElement::compile(util::sref<proto::Block>
                                                       , util::sref<SymbolTable>) const
{
    return util::mkptr(new proto::ListElement(pos));
}

std::string ListElement::typeName() const
{
    return "list element";
}

util::sptr<Expression const> ListElement::fold() const
{
    return util::mkptr(new ListElement(pos));
}

util::sptr<proto::Expression const> ListIndex::compile(util::sref<proto::Block>
                                                     , util::sref<SymbolTable>) const
{
    return util::mkptr(new proto::ListIndex(pos));
}

std::string ListIndex::typeName() const
{
    return "list index";
}

util::sptr<Expression const> ListIndex::fold() const
{
    return util::mkptr(new ListIndex(pos));
}

util::sptr<proto::Expression const> ListAppend::compile(util::sref<proto::Block> block
                                                      , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::ListAppend(pos
                                           , lhs->compile(block, st)
                                           , rhs->compile(block, st)));
}

std::string ListAppend::typeName() const
{
    return '(' + lhs->typeName() + "++" + rhs->typeName() + ')';
}

util::sptr<Expression const> ListAppend::fold() const
{
    return util::mkptr(new ListAppend(pos, lhs->fold(), rhs->fold()));
}

util::sptr<proto::Expression const> Call::compile(util::sref<proto::Block> block
                                                , util::sref<SymbolTable> st) const
{
    return st->compileCall(pos, block, name, args);
}

std::string Call::typeName() const
{
    if (args.empty()) {
        return "(call(" + name + "))";
    }
    std::string args_names;
    std::for_each(args.begin()
                , args.end()
                , [&](util::sptr<Expression const> const& arg)
                  {
                      args_names += (arg->typeName() + ", ");
                  });
    return "(call(" + name + ")(" + args_names.substr(0, args_names.length() - 2) + "))";
}

util::sptr<Expression const> Call::fold() const
{
    return foldCall();
}

util::sptr<Call const> Call::foldCall() const
{
    return util::mkptr(new Call(pos, name, foldList(args)));
}

util::sptr<proto::Expression const> MemberCall::compile(util::sref<proto::Block> block
                                                      , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::MemberCall(pos
                                           , object->compile(block, st)
                                           , call->name
                                           , compileList(call->args, block, st)));
}

std::string MemberCall::typeName() const
{
    return "(member (" + object->typeName() + "))." + call->typeName();
}

util::sptr<Expression const> MemberCall::fold() const
{
    return util::mkptr(new MemberCall(pos, object->fold(), call->foldCall()));
}

util::sptr<proto::Expression const> FuncReference::compile(util::sref<proto::Block> block
                                                         , util::sref<SymbolTable> st) const
{
    return util::mkptr(new proto::FuncReference(pos, st->queryFunc(pos, name, param_count)
                                                       ->compile(block)));
}

std::string FuncReference::typeName() const
{
    return "(func reference(" + name + '@' + util::str(param_count) + "))";
}

util::sptr<Expression const> FuncReference::fold() const
{
    return util::mkptr(new FuncReference(pos, name, param_count));
}
