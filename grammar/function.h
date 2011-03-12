#ifndef __STACKENING_GRAMMAR_FUNCTION_H__
#define __STACKENING_GRAMMAR_FUNCTION_H__

#include <string>
#include <vector>

#include "block.h"
#include "../util/pointer.h"
#include "../misc/pos-type.h"

namespace grammar {

    struct function {
        function(misc::pos_type const& ps
               , std::string const& func_name
               , std::vector<std::string> const& params
               , block func_body)
            : pos(ps)
            , name(func_name)
            , param_names(params)
            , body(std::move(func_body))
        {}

        void compile(util::sref<flchk::filter> filter) const;

        misc::pos_type const pos;
        std::string const name;
        std::vector<std::string> const param_names;
        block const body;
    };

}

#endif /* __STACKENING_GRAMMAR_FUNCTION_H__ */