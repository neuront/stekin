#ifndef __STACKENING_PARSER_TEST_DATA_TREES_H__
#define __STACKENING_PARSER_TEST_DATA_TREES_H__

#include <list>
#include <string>
#include <ostream>

#include "data-node.h"
#include "../../util/pointer.h"

namespace test {

    struct data_tree {
        data_tree& operator()(misc::pos_type const& pos
                            , data_node_base::node_type type
                            , std::string const& data);
        data_tree& operator()(misc::pos_type const& pos
                            , int indent
                            , data_node_base::node_type type
                            , std::string const& data);
        data_tree& operator()(misc::pos_type const& pos, data_node_base::node_type type);
        data_tree& operator()(misc::pos_type const& pos, int indent, data_node_base::node_type type);
    private:
        data_tree() {}
    public:
        static data_tree& expect_one();
        static data_tree& actual_one();

        static void verify();
    private:
        std::list<util::sptr<data_node_base const>> _nodes;
    };

}

std::ostream& operator<<(std::ostream& os, util::sptr<test::data_node_base const> const& node);

#endif /* __STACKENING_PARSER_TEST_DATA_TREES_H__ */
