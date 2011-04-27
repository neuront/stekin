#ifndef __STEKIN_FLOWCHECK_FUNCTION_H__
#define __STEKIN_FLOWCHECK_FUNCTION_H__

#include <string>
#include <vector>

#include "block.h"
#include "../util/pointer.h"
#include "../misc/pos-type.h"

namespace flchk {

    struct Function {
        Function(misc::position const& ps
               , std::string const& func_name
               , std::vector<std::string> const& params
               , Block func_body
               , bool func_contains_void_return)
            : pos(ps)
            , name(func_name)
            , param_names(params)
            , body(std::move(func_body))
            , contains_void_return(func_contains_void_return)
        {}

        void compile(util::sref<proto::Scope> scope) const;

        misc::position const pos;
        std::string const name;
        std::vector<std::string> const param_names;
        Block const body;
        bool const contains_void_return;
    };

}

#endif /* __STEKIN_FLOWCHECK_FUNCTION_H__ */
