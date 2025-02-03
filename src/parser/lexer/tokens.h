#include <string>
#include <ostream>

#ifndef TOKENS_H
#define TOKENS_H

namespace Compiler::Lexing {

enum TokenType {
    // File related
    START_T,
    EOF_T,

    // Keywords
    SELECT_T,
    FROM_T,
    WHERE_T,
    AS_T,
    GROUP_T,
    BY_T,
    HAVING_T,
    ORDER_T,
    JOIN_T,
    ON_T,
    LEFT_T,
    RIGHT_T,
    INNER_T,
    OUTER_T,
    SET_T,
    INSERT_T,
    INTO_T,
    DELETE_T,
    CREATE_T,
    DROP_T,
    TABLE_T,
    // Operators
    EQ_OP_T,
    PUNCT_T,
    OP_T,
    ENDL_T,

    // Expression
    STRING_LITT_T,
    NUM_LITT_T,

    // Identifiers
    VAR_NAME_T,
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

std::ostream& operator<<(std::ostream& os, const Token& tok);

} // namespace Compiler::Lexing

#endif // !TOKENS_H
