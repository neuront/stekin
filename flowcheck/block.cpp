#include <algorithm>

#include "block.h"
#include "function.h"
#include "node-base.h"
#include "../proto/node-base.h"
#include "../proto/scope.h"
#include "../proto/function.h"

using namespace flchk;

void block::add_stmt(util::sptr<Statement const> stmt)
{
    _stmts.push_back(std::move(stmt));
}

void block::def_func(misc::pos_type const& pos
                   , std::string const& name
                   , std::vector<std::string> const& param_names
                   , block body
                   , bool contains_void_return)
{
    _funcs.push_back(std::move(util::mkptr(new function(pos
                                                      , name
                                                      , param_names
                                                      , std::move(body)
                                                      , contains_void_return))));
}

void block::compile(util::sref<proto::scope> scope) const 
{
    std::vector<util::sref<proto::function>> decls;
    decls.reserve(_funcs.size());
    std::for_each(_funcs.begin()
                , _funcs.end()
                , [&](util::sptr<function const> const& func)
                  {
                      decls.push_back(func->declare(scope));
                  });

    std::vector<util::sref<proto::function>>::iterator func_decl_iter = decls.begin();
    std::for_each(_funcs.begin()
                , _funcs.end()
                , [&](util::sptr<function const> const& func)
                  {
                      func->body.compile(*func_decl_iter++);
                  });

    std::for_each(_stmts.begin()
                , _stmts.end()
                , [&](util::sptr<Statement const> const& stmt)
                  {
                      scope->add_stmt(stmt->compile(scope));
                  });
}

void block::append(block following)
{
    std::for_each(following._funcs.begin()
                , following._funcs.end()
                , [&](util::sptr<function const>& func)
                  {
                      _funcs.push_back(std::move(func));
                  });

    std::for_each(following._stmts.begin()
                , following._stmts.end()
                , [&](util::sptr<Statement const>& stmt)
                  {
                      _stmts.push_back(std::move(stmt));
                  });
}
