#include <algorithm>

#include "node-base.h"
#include "block.h"
#include "../output/statement-writer.h"

using namespace inst;

void block::add_stmt(util::sptr<Statement const> stmt)
{
    _stmts.push_back(std::move(stmt));
}

void block::write() const
{
    output::block_begin();
    std::for_each(_stmts.begin()
                , _stmts.end()
                , [&](util::sptr<Statement const> const& stmt)
                  {
                      stmt->write();
                  });
    output::block_end();
}
