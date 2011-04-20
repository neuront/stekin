#ifndef __STEKIN_PARSER_SYNTAX_TYPE_H__
#define __STEKIN_PARSER_SYNTAX_TYPE_H__

#include <string>
#include <vector>

#include "../grammar/fwd-decl.h"
#include "../report/errors.h"
#include "../util/pointer.h"
#include "../misc/pos-type.h"

namespace parser {

    struct op_img {
        std::string const img;

        explicit op_img(char const* image)
            : img(image)
        {}
    };

    struct identifier {
        misc::pos_type const pos;
        std::string const id;

        identifier(misc::pos_type const& ps, char const* id_text)
            : pos(ps)
            , id(id_text)
        {}
    };

    struct param_names {
        param_names* add(std::string const& name)
        {
            _names.push_back(name);
            return this;
        }

        std::vector<std::string> get() const
        {
            return _names;
        }
    private:
        std::vector<std::string> _names;
    };

    struct arg_list {
        arg_list* add(grammar::expr_base const* expr)
        {
            _params.push_back(std::move(util::mkptr(expr)));
            return this;
        }

        std::vector<util::sptr<grammar::expr_base const>> deliver_args()
        {
            return std::move(_params);
        }
    private:
        std::vector<util::sptr<grammar::expr_base const>> _params;
    };

}

#endif /* __STEKIN_PARSER_SYNTAX_TYPE_H__ */
