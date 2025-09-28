#include <gtest/gtest.h>

#include "../src/utils/stack.h"

using namespace Database::Utils;

class StackTest : public testing::Test {
protected:
    void SetUp() override
    {
        l0 = Stack<int>();
        l1 = Stack<int>();

        l0.push(0);
        l0.push(1);
        l0.push(2);
        l0.push(3);
        l0.push(4);
    }

    Stack<int> l0;
    Stack<int> l1;
};

TEST_F(StackTest, IsEmptyInitially)
{
    ASSERT_TRUE(l1.empty());
}

TEST_F(StackTest, GetValues)
{
    ASSERT_EQ(4, l0.pop());
    ASSERT_EQ(3, l0.pop());
    ASSERT_EQ(2, l0.pop());
    ASSERT_EQ(1, l0.pop());
    ASSERT_EQ(0, l0.pop());

    ASSERT_TRUE(l0.empty());
}

TEST_F(StackTest, InsertValues)
{
    l1.push(17);

    ASSERT_EQ(17, l1.pop());

    ASSERT_TRUE(l1.empty());
}
