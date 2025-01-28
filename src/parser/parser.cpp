#include "parser.h"

#include "parser/ast_types.h"

namespace Compiler::Parsing {

Parser::Parser(Lexing::TokenType file_start)
    : tree(Tree(file_start))
{
}

void Parser::parse(std::vector<Lexing::Token> tokens)
{
    if (tokens.empty()) {
        return;
    }

    switch (tokens.front().token) {

    default:
        break;
    }
}

} // namespace Compiler::Parsing
