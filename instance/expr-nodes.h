#ifndef __STEKIN_INSTANCE_EXPRESSION_NODES_H__
#define __STEKIN_INSTANCE_EXPRESSION_NODES_H__

#include <vector>

#include "node-base.h"
#include "variable.h"
#include "fwd-decl.h"
#include "../proto/func-reference-type.h"
#include "../util/pointer.h"
#include "../misc/platform.h"

namespace inst {

    struct IntLiteral
        : public Expression
    {
        explicit IntLiteral(platform::i4_type v)
            : value(v)
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        platform::i4_type const value;
    };

    struct FloatLiteral
        : public Expression
    {
        explicit FloatLiteral(platform::f8_type v)
            : value(v)
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        platform::f8_type const value;
    };

    struct BoolLiteral
        : public Expression
    {
        explicit BoolLiteral(bool v)
            : value(v)
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        bool const value;
    };

    struct Reference
        : public Expression
    {
        explicit Reference(Variable const& v)
            : var(v)
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        Variable const var;
    };

    struct Call
        : public Expression
    {
        Call(util::sref<proto::FuncInstDraft const> f, std::vector<util::sptr<Expression const>> a)
            : func(f)
            , args(std::move(a))
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        util::sref<proto::FuncInstDraft const> const func;
        std::vector<util::sptr<Expression const>> args;
    };

    struct FuncReference
        : public Expression
    {
        explicit FuncReference(util::sref<proto::FuncReferenceType const> t)
            : type(t)
        {}

        util::sref<Type const> typeof() const;
        void write() const;
        util::sref<proto::FuncReferenceType const> const type;
    };

    struct BinaryOp
        : public Expression
    {
        BinaryOp(util::sptr<Expression const> l
               , std::string const& o
               , util::sref<Type const> t
               , util::sptr<Expression const> r)
            : lhs(std::move(l))
            , op(o)
            , type(t)
            , rhs(std::move(r))
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        util::sptr<Expression const> const lhs;
        std::string const op;
        util::sref<Type const> const type;
        util::sptr<Expression const> const rhs;
    };

    struct PreUnaryOp
        : public Expression
    {
        PreUnaryOp(std::string const& o, util::sref<Type const> t , util::sptr<Expression const> r)
            : op(o)
            , type(t)
            , rhs(std::move(r))
        {}

        util::sref<Type const> typeof() const;
        void write() const;

        std::string const op;
        util::sref<Type const> type;
        util::sptr<Expression const> const rhs;
    };

    struct Conjunction
        : public Expression
    {
        Conjunction(misc::position const& p
                  , util::sptr<Expression const> l
                  , util::sptr<Expression const> r);

        util::sref<Type const> typeof() const;
        void write() const;

        util::sptr<Expression const> const lhs;
        util::sptr<Expression const> const rhs;
    };

    struct Disjunction
        : public Expression
    {
        Disjunction(misc::position const& p
                  , util::sptr<Expression const> l
                  , util::sptr<Expression const> r);

        util::sref<Type const> typeof() const;
        void write() const;

        util::sptr<Expression const> const lhs;
        util::sptr<Expression const> const rhs;
    };

    struct Negation
        : public Expression
    {
        Negation(misc::position const& p, util::sptr<Expression const> r);

        util::sref<Type const> typeof() const;
        void write() const;

        util::sptr<Expression const> const rhs;
    };

}

#endif /* __STEKIN_INSTANCE_EXPRESSION_NODES_H__ */
