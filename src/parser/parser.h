#include "parser/ast_types.h"
#include "lexer/tokens.h"

#include <vector>

#ifndef PARSER_H
#define PARSER_H

namespace Compiler::Parsing {

class Parser {
private:
  Tree tree;
public:
    Parser(Lexing::TokenType file_start);
    ~Parser();

    void parse(std::vector<Lexing::Token>);
};
} // namespace Compiler::Parsing

#endif // !PARSER_H
