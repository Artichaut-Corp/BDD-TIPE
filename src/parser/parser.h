#include "lexer/tokenizer.h"
#include "lexer/tokens.h"
#include "parser/ast_types.h"

#include <vector>

#ifndef PARSER_H
#define PARSER_H

namespace Compiler::Parsing {

class Parser {
private:
    Tree tree;
    Lexing::Tokenizer t;

public:
    Parser(Lexing::TokenType file_start);
    ~Parser();

    void parse();
};
} // namespace Compiler::Parsing

#endif // !PARSER_H
