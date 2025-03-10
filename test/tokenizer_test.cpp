#include <gtest/gtest.h>

#include "../src/lexer/lexer.h"
#include "../src/lexer/tokenizer.h"

using namespace Database::Lexing;

class TokenizerTest : public testing::Test {
protected:
    void SetUp() override
    {
    }
};

TEST(TokenizerTest, TestIsEmpty)
{

    Database::Lexing::Tokenizer t = Tokenizer("DELETE FROM ville;", 0);
    ASSERT_FALSE(t.isEmpty());
}

TEST(TokenizerTest, TestPeek)
{

    Database::Lexing::Tokenizer t = Tokenizer("DELETE FROM ville;", 0);
    ASSERT_EQ(DELETE_T, t.peek().m_Token);
}

TEST(TokenizerTest, TestNext)
{

    Database::Lexing::Tokenizer t = Tokenizer("DELETE FROM ville;", 0);

    ASSERT_EQ(DELETE_T, t.next().m_Token);
    ASSERT_EQ(FROM_T, t.next().m_Token);
}
