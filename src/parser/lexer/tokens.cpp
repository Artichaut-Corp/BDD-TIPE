#include <ostream>

#include "tokens.h"

namespace Compiler::Lexing {

Token::Token(TokenType type, std::string value)
{
    this->token = type;
    this->value = value;
}

void Token::create_token(const TokenType type, const std::string& value)
{
    this->token = type;
    this->value = value;

    return;
}

void Token::create_token(const TokenType type)
{
    this->token = type;
    this->value = std::string();

    return;
}

void Token::print() { std::printf("%d:%s ", token, value.c_str()); }

std::ostream& operator<<(std::ostream& os, const Token& t)
{
    os << "[" << t.token << "::" << (t.value == "" ? "NONE" : t.value) << "]";

    return os;
}

} // namespace Compiler::Lexing
