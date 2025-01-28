#include <string>

#ifndef TOKENS_H
#define TOKENS_H

namespace Compiler::Lexing {

enum TokenType {
    // TBD

};

class Token {
public:
    TokenType token;
    std::string value;

    void create_token(const TokenType type, const std::string& value);

    void create_token(const TokenType type);

    void print();

    Token() = default;

    Token(TokenType type, std::string value = "");
};

} // namespace Compiler::Lexing

#endif // !TOKENS_H
