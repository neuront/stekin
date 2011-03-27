#ifndef __STACKENING_PROTO_SYMBOL_TABLE_H__
#define __STACKENING_PROTO_SYMBOL_TABLE_H__

#include <string>
#include <map>
#include <list>
#include <vector>

#include "fwd-decl.h"
#include "../instance/fwd-decl.h"
#include "../util/pointer.h"
#include "../misc/pos-type.h"

namespace proto {

    struct func_signature {
        std::string const name;
        int const param_count;

        func_signature(std::string const& n, int const pc)
            : name(n)
            , param_count(pc)
        {}

        bool operator<(func_signature const& rhs) const
        {
            return name == rhs.name ? param_count < rhs.param_count : name < rhs.name;
        }
    };

    struct symbol_table {
        int const level;

        explicit symbol_table(util::sref<symbol_table const> ext_symbols)
            : level(ext_symbols->level + 1)
            , _external_or_null_on_global(ext_symbols)
        {}

        symbol_table()
            : level(0)
            , _external_or_null_on_global(NULL)
        {}

        symbol_table(symbol_table&& rhs)
            : level(rhs.level)
            , _external_var_refs(rhs._external_var_refs)
            , _var_defs(rhs._var_defs)
            , _funcs(rhs._funcs)
            , _func_entities(std::move(rhs._func_entities))
            , _external_or_null_on_global(rhs._external_or_null_on_global)
        {}

        void ref_var(misc::pos_type const& pos, std::string const& name);
        void def_var(misc::pos_type const& pos, std::string const& name);
        util::sref<function> def_func(misc::pos_type const& pos
                                    , std::string const& name
                                    , std::vector<std::string> const& param_names
                                    , bool hint_void_return);
        util::sptr<expr_base const> query_call(misc::pos_type const& pos
                                             , std::string const& name
                                             , std::vector<util::sptr<expr_base const>> args) const;
        util::sref<function> query_func(misc::pos_type const& pos
                                      , std::string const& name
                                      , int param_count) const;
        std::map<std::string, inst::variable const>
            bind_external_var_refs(misc::pos_type const& pos, util::sref<inst::scope const> ext_scope) const;
    private:
        std::map<std::string, std::list<misc::pos_type>> _external_var_refs;
        std::map<std::string, misc::pos_type> _var_defs;
        std::map<func_signature, util::sref<function> const> _funcs;
        std::list<function> _func_entities;
        util::sref<symbol_table const> const _external_or_null_on_global;

        util::sref<function> _query_func_or_null_if_nonexist(std::string const& name, int param_count) const;
        util::sref<function> _query_func_in_external_or_null_if_nonexist(std::string const& name
                                                                       , int param_count) const;

        static function _fake_prototype;
    };

}

#endif /* __STACKENING_PROTO_SYMBOL_TABLE_H__ */
