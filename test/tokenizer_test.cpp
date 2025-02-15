#include <gtest/gtest.h>

#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/tokenizer.h"

using namespace Compiler::Lexing;

class TokenizerTest : public testing::Test {
protected:
    void SetUp() override
    {
    }
};

TEST(TokenizerTest, TestIsEmpty)
{

    Compiler::Lexing::Tokenizer t = Tokenizer("DELETE FROM ville;", 0);
    ASSERT_FALSE(t.isEmpty());
}

TEST(TokenizerTest, TestPeek)
{

    Compiler::Lexing::Tokenizer t = Tokenizer("DELETE FROM ville;", 0);
    ASSERT_EQ(DELETE_T, t.peek().m_Token);
}

TEST(TokenizerTest, TestNext)
{

    Compiler::Lexing::Tokenizer t = Tokenizer("DELETE FROM ville;", 0);
    ASSERT_EQ(DELETE_T, t.next().m_Token);
    ASSERT_EQ(FROM_T, t.next().m_Token);
}
