#include <ostream>
#include <string>

#ifndef TOKENS_H
#define TOKENS_H

namespace Compiler::Lexing {

enum TokenType {
    // File related
    START_T,
    EOF_T,

    // Keywords

    // DML
    SELECT_T,
    UPDATE_T,
    INSERT_T,
    DELETE_T,
    FROM_T,
    SET_T,
    INTO_T,
    WHERE_T,
    GROUP_T,
    BY_T,
    HAVING_T,
    ORDER_T,
    ASC_T,
    DESC_T,
    LIMIT_T,
    OFFSET_T,
    VALUES_T,
    DEFAULT_T,
    
    // Aggregate Functions
    AGGR_FUNC_T,

    // Joins
    JOIN_T,
    ON_T,
    LEFT_T,
    RIGHT_T,
    INNER_T,
    OUTER_T,
    FULL_T,
    CROSS_T,

    // DDL
    CREATE_T,
    RENAME_T,
    DROP_T,
    ALTER_T,
    TABLE_T,
    DATABASE_T,
    UNIQUE_T,
    NULL_T,
    FOREIGN_T,
    PRIMARY_T,
    KEY_T,

    // Operators
    IS_T,
    NOT_T,
    DISTINCT_T,
    AND_T,
    OR_T,
    EQ_OP_T,
    MATH_OP_T,
    COMMA_T,
    SEMI_COLON_T,
    DOT_T,
    ENDL_T,
    LPAREN_T,
    RPAREN_T,

    // Expression
    STRING_LITT_T,
    NUM_LITT_T,

    // Identifiers
    VAR_NAME_T,
};

class Token {
public:
    TokenType m_Token;
    std::string m_Value;

    void createToken(const TokenType type, const std::string& value);

    void createToken(const TokenType type);

    void print();

    Token() = default;

    Token(TokenType type, std::string value = "");
};

std::ostream& operator<<(std::ostream& os, const Token& tok);

} // namespace Compiler::Lexing

#endif // !TOKENS_H
