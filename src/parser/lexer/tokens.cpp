#include <ostream>

#include "tokens.h"

namespace Compiler::Lexing {

Token::Token(TokenType type, std::string value)
{
    this->m_Token = type;
    this->m_Value = value;
}

void Token::createToken(const TokenType type, const std::string& value)
{
    this->m_Token = type;
    this->m_Value = value;

    return;
}

void Token::createToken(const TokenType type)
{
    this->m_Token = type;
    this->m_Value = std::string();

    return;
}

void Token::print() { std::printf("%d:%s ", m_Token, m_Value.c_str()); }

std::ostream& operator<<(std::ostream& os, const Token& tok)
{
    os << "(" ;

    os << tok.m_Token << " | ";

    std::string val = tok.m_Value;
    val == "" ? os << "NONE" : os << val;

    os << ")";

    return os;
}


} // namespace Compiler::Lexing
