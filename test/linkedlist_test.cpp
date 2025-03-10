#include <gtest/gtest.h>
#include <memory>

#include "../src/utils/linked_list.h"

using namespace Database::Utils;

class LinkedListTest : public testing::Test {
protected:
    void SetUp() override
    {
        l0 = std::unique_ptr<LinkedList<int>>(new LinkedList<int>());
        l1 = LinkedList<int>();

        l0->append(0);
        l0->append(1);
        l0->append(2);
        l0->append(3);
        l0->append(4);
    }

    std::unique_ptr<LinkedList<int>> l0;
    LinkedList<int> l1;
};

TEST_F(LinkedListTest, IsEmptyInitially)
{
    ASSERT_TRUE(l1.is_empty());
}

TEST_F(LinkedListTest, GetValues)
{
    ASSERT_EQ(4, l0->get_first().value());
    ASSERT_EQ(0, l0->get_at(4).value());
}

TEST_F(LinkedListTest, AdvanceAndDiscard)
{
    l0->advance();
    ASSERT_EQ(3, l0->get_first());
}

TEST_F(LinkedListTest, EmptyList)
{
    ASSERT_FALSE(l0->is_empty());
    while (!l0->is_empty()) {
        l0.get()->advance();
    }

    ASSERT_TRUE(l0.get()->is_empty());
}
