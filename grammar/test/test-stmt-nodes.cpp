#include <list>
#include <gtest/gtest.h>

#include "test-common.h"
#include "../stmt-nodes.h"
#include "../expr-nodes.h"
#include "../function.h"
#include "../../flowcheck/node-base.h"
#include "../../flowcheck/filter.h"
#include "../../flowcheck/function.h"
#include "../../proto/node-base.h"
#include "../../test/phony-errors.h"

using namespace test;

typedef GrammarTest StmtNodesTest;

TEST_F(StmtNodesTest, Arithmetics)
{
    misc::position pos(1);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));
    grammar::Arithmetics arith0(pos, std::move(util::mkptr(new grammar::IntLiteral(pos, "1840"))));
    grammar::Arithmetics arith1(pos, std::move(util::mkptr(new grammar::BoolLiteral(pos, false))));
    arith0.compile(*filter);
    arith1.compile(*filter);
    filter->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, ARITHMETICS)
            (pos, INTEGER, "1840")
        (pos, ARITHMETICS)
            (pos, BOOLEAN, "false")
        (BLOCK_END)
    ;
}

TEST_F(StmtNodesTest, VarDef)
{
    misc::position pos(2);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));
    grammar::VarDef def0(pos, "Shinji", std::move(
                                            util::mkptr(new grammar::FloatLiteral(pos, "18.47"))));
    grammar::VarDef def1(pos, "Asuka", std::move(
                                            util::mkptr(new grammar::Reference(pos, "tsundere"))));
    def0.compile(*filter);
    def1.compile(*filter);
    filter->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, VAR_DEF, "Shinji")
            (pos, FLOATING, "18.47")
        (pos, VAR_DEF, "Asuka")
            (pos, REFERENCE, "tsundere")
        (BLOCK_END)
    ;
}

TEST_F(StmtNodesTest, Returns)
{
    misc::position pos(3);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));
    grammar::Return ret0(pos, std::move(util::mkptr(new grammar::Reference(pos, "KaworuNagisa"))));
    grammar::ReturnNothing ret1(pos);
    ret0.compile(*filter);
    ret1.compile(*filter);
    filter->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, RETURN)
            (pos, REFERENCE, "KaworuNagisa")
        (pos, RETURN_NOTHING)
        (BLOCK_END)
    ;
}

TEST_F(StmtNodesTest, Block)
{
    misc::position pos(4);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));
    grammar::Block block;
    block.addStmt(std::move(
                util::mkptr(new grammar::VarDef(pos, "Misato", std::move(
                            util::mkptr(new grammar::Reference(pos, "Katsuragi")))))));
    block.addStmt(std::move(util::mkptr(new grammar::ReturnNothing(pos))));
    block.compile(std::move(filter))->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, VAR_DEF, "Misato")
            (pos, REFERENCE, "Katsuragi")
        (pos, RETURN_NOTHING)
        (BLOCK_END)
    ;
}

TEST_F(StmtNodesTest, Branch)
{
    misc::position pos(6);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));
    grammar::Branch(pos
                  , std::move(util::mkptr(new grammar::BoolLiteral(pos, true)))
                  , std::move(grammar::Block())
                  , std::move(grammar::Block()))
        .compile(*filter);

    grammar::Block block0;
    block0.addStmt(std::move(
                        util::mkptr(new grammar::Arithmetics(pos, std::move(
                                        util::mkptr(new grammar::Reference(pos, "Kaji")))))));
    block0.addStmt(std::move(util::mkptr(new grammar::ReturnNothing(pos))));
    grammar::BranchConsqOnly(pos
                           , std::move(util::mkptr(new grammar::BoolLiteral(pos, false)))
                           , std::move(block0))
        .compile(*filter);

    grammar::Block block1;
    block1.addStmt(std::move(
                util::mkptr(new grammar::Arithmetics(pos, std::move(
                            util::mkptr(new grammar::Reference(pos, "Ryoji")))))));
    block1.addStmt(std::move(util::mkptr(new grammar::ReturnNothing(pos))));
    grammar::BranchAlterOnly(pos
                           , std::move(util::mkptr(new grammar::BoolLiteral(pos, true)))
                           , std::move(block1))
        .compile(*filter);

    grammar::Block block2;
    block2.addStmt(std::move(
                util::mkptr(new grammar::Arithmetics(pos, std::move(
                            util::mkptr(new grammar::IntLiteral(pos, "7")))))));
    block2.addStmt(std::move(util::mkptr(new grammar::ReturnNothing(pos))));
    grammar::Block block3;
    block3.addStmt(std::move(
                util::mkptr(new grammar::Return(pos, std::move(
                            util::mkptr(new grammar::Reference(pos, "betsuni")))))));
    grammar::Branch(pos
                  , std::move(util::mkptr(new grammar::BoolLiteral(pos, false)))
                  , std::move(block2)
                  , std::move(block3))
        .compile(*filter);
    filter->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, BRANCH)
        (pos, BOOLEAN, "true")
        (CONSEQUENCE)
            (BLOCK_BEGIN)
            (BLOCK_END)
        (ALTERNATIVE)
            (BLOCK_BEGIN)
            (BLOCK_END)

        (pos, BRANCH_CONSQ_ONLY)
        (pos, BOOLEAN, "false")
        (CONSEQUENCE)
            (BLOCK_BEGIN)
            (pos, ARITHMETICS)
                (pos, REFERENCE, "Kaji")
            (pos, RETURN_NOTHING)
            (BLOCK_END)

        (pos, BRANCH_ALTER_ONLY)
        (pos, BOOLEAN, "true")
        (ALTERNATIVE)
            (BLOCK_BEGIN)
            (pos, ARITHMETICS)
                (pos, REFERENCE, "Ryoji")
            (pos, RETURN_NOTHING)
            (BLOCK_END)

        (pos, BRANCH)
        (pos, BOOLEAN, "false")
        (CONSEQUENCE)
            (BLOCK_BEGIN)
            (pos, ARITHMETICS)
                (pos, INTEGER, "7")
            (pos, RETURN_NOTHING)
            (BLOCK_END)
        (ALTERNATIVE)
            (BLOCK_BEGIN)
            (pos, RETURN)
                (pos, REFERENCE, "betsuni")
            (BLOCK_END)
        (BLOCK_END)
    ;
}

TEST_F(StmtNodesTest, Functions)
{
    misc::position pos(8);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));
    grammar::Function func0(pos, "func0", std::vector<std::string>(), std::move(grammar::Block()));
    func0.compile(*filter);

    grammar::Block body;
    body.addStmt(std::move(
                util::mkptr(new grammar::Arithmetics(pos, std::move(
                            util::mkptr(new grammar::Reference(pos, "Kuroi")))))));
    body.addStmt(std::move(util::mkptr(new grammar::ReturnNothing(pos))));
    grammar::Function func1(pos
                          , "func1"
                          , std::vector<std::string>({ "Konata", "Kagami", "Tsukasa", "Miyuki" })
                          , std::move(body));
    func1.compile(*filter);
    filter->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, FUNC_DEF, "func0")
            (BLOCK_BEGIN)
            (BLOCK_END)
        (pos, FUNC_DEF, "func1")
            (pos, PARAMETER, "Konata")
            (pos, PARAMETER, "Kagami")
            (pos, PARAMETER, "Tsukasa")
            (pos, PARAMETER, "Miyuki")
            (BLOCK_BEGIN)
            (pos, ARITHMETICS)
                (pos, REFERENCE, "Kuroi")
            (pos, RETURN_NOTHING)
            (BLOCK_END)
        (BLOCK_END)
    ;
}

TEST_F(StmtNodesTest, Mixed)
{
    misc::position pos(9);
    util::sptr<flchk::Filter> filter(std::move(mkfilter()));

    grammar::Block block_nested;
    block_nested.addStmt(std::move(
                util::mkptr(new grammar::Arithmetics(pos, std::move(
                            util::mkptr(new grammar::IntLiteral(pos, "9")))))));
    util::sptr<grammar::Function> func_nested0(new grammar::Function(
                                                        pos
                                                      , "funcn"
                                                      , std::vector<std::string>({ "SOS" })
                                                      , std::move(block_nested)));
    util::sptr<grammar::Function> func_nested1(new grammar::Function(
                                                        pos
                                                      , "funcn"
                                                      , std::vector<std::string>()
                                                      , std::move(grammar::Block())));

    grammar::Block body;
    body.addStmt(std::move(
                util::mkptr(new grammar::Arithmetics(pos, std::move(
                            util::mkptr(new grammar::Reference(pos, "Kyon")))))));
    body.addFunc(std::move(func_nested0));
    body.addFunc(std::move(func_nested1));
    body.addStmt(std::move(util::mkptr(new grammar::ReturnNothing(pos))));

    grammar::Function func(pos
                         , "funco"
                         , std::vector<std::string>({ "Suzumiya", "Koizumi", "Nagato", "Asahina" })
                         , std::move(body));
    func.compile(*filter);
    filter->deliver().compile(nulscope);

    DataTree::expectOne()
        (BLOCK_BEGIN)
        (pos, FUNC_DEF, "funco")
            (pos, PARAMETER, "Suzumiya")
            (pos, PARAMETER, "Koizumi")
            (pos, PARAMETER, "Nagato")
            (pos, PARAMETER, "Asahina")
            (BLOCK_BEGIN)
            (pos, FUNC_DEF, "funcn")
                (pos, PARAMETER, "SOS")
                (BLOCK_BEGIN)
                (pos, ARITHMETICS)
                    (pos, INTEGER, "9")
                (BLOCK_END)
            (pos, FUNC_DEF, "funcn")
                (BLOCK_BEGIN)
                (BLOCK_END)
            (pos, ARITHMETICS)
                (pos, REFERENCE, "Kyon")
            (pos, RETURN_NOTHING)
            (BLOCK_END)
        (BLOCK_END)
    ;
}
