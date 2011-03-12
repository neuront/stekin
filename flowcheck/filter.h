#ifndef __STACKENING_FLOW_CHECK_FILTER_H__
#define __STACKENING_FLOW_CHECK_FILTER_H__

#include <string>
#include <vector>

#include "accumulator.h"
#include "../misc/pos-type.h"

namespace flchk {

    struct filter {
        filter() = default;
        filter(filter const&) = delete;

        filter(filter&& rhs)
            : _accumulator(std::move(rhs._accumulator))
        {}

        virtual ~filter() {}
    public:
        void add_func_ret(misc::pos_type const& pos, util::sptr<expr_base const> ret_val);
        void add_func_ret_nothing(misc::pos_type const& pos);
        void add_arith(misc::pos_type const& pos, util::sptr<expr_base const> expr);

        void add_branch(misc::pos_type const& pos
                      , util::sptr<expr_base const> predicate
                      , util::sptr<filter> consequence
                      , util::sptr<filter> alternative);

        void add_branch(misc::pos_type const& pos
                      , util::sptr<expr_base const> predicate
                      , util::sptr<filter> consequence);

        void add_branch_alt_only(misc::pos_type const& pos
                               , util::sptr<expr_base const> predicate
                               , util::sptr<filter> alternative);

        virtual void def_var(misc::pos_type const& pos
                           , std::string const& name
                           , util::sptr<expr_base const> init);

        virtual void def_func(misc::pos_type const& pos
                            , std::string const& name
                            , std::vector<std::string> const& param_names
                            , util::sptr<filter> body);
    public:
        block deliver();
    protected:
        accumulator _accumulator;
    };

    struct symbol_def_filter
        : public filter
    {
        void def_var(misc::pos_type const& pos, std::string const& name, util::sptr<expr_base const>);

        void def_func(misc::pos_type const& pos
                    , std::string const& name
                    , std::vector<std::string> const&
                    , filter);
    };

}

#endif /* __STACKENING_FLOW_CHECK_FILTER_H__ */