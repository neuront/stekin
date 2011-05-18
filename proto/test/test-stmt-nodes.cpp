#include <gtest/gtest.h>

#include "test-common.h"
#include "../stmt-nodes.h"
#include "../expr-nodes.h"
#include "../block.h"
#include "../function.h"
#include "../func-reference-type.h"
#include "../symbol-table.h"
#include "../../test/common.h"
#include "../../test/phony-errors.h"

using namespace test;

struct StmtNodesTest
    : public ProtoTest
{
    void SetUp()
    {
        ProtoTest::SetUp();
        global_st.reset(new proto::SymbolTable);
    }

    util::sptr<proto::SymbolTable> global_st;
};

TEST_F(StmtNodesTest, BranchConditionTypeCheck)
{
    misc::position pos(1);
    misc::position pos_d(100);
    global_st->defVar(pos_d, proto::Type::BIT_INT, "kokopelli");
    util::sptr<proto::Scope> scope(new proto::Scope);

    util::sref<proto::Function> func(scope->declare(pos_d, "f", std::vector<std::string>(), true));
    func->setFreeVariables(std::vector<std::string>{ "kokopelli" });
    func->addStmt(util::mkptr(new proto::Branch(pos
                                              , util::mkptr(new proto::Reference(pos, "kokopelli"))
                                              , util::mkptr(new proto::Block)
                                              , util::mkptr(new proto::Block))));
    proto::Call call(pos_d, func, std::vector<util::sptr<proto::Expression const>>());
    call.inst(*global_st);

    ASSERT_EQ(1, getCondNotBools().size());
    ASSERT_EQ(pos, getCondNotBools()[0].pos);
    ASSERT_EQ(proto::Type::BIT_INT->name(), getCondNotBools()[0].type_name);
}