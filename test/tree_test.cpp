#include <gtest/gtest.h>
#include <sys/stat.h>

#include "../src/parser/parser/tree.h"

#include "../src/parser/parser/ast_node.h"

class TreeTest : public testing::Test {
protected:
    void SetUp() override
    {
        tree.(Add);
        tree.append(1);
        tree.append(2);
    }

    Compiler::Parsing::Tree tree;
};
}