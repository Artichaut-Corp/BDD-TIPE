#include "parser.h"

#include "parser/ast_types.h"
#include <memory>
#include <string>

namespace Compiler::Parsing {

Parser::Parser(Lexing::TokenType file_start, std::string input)
    : tree(Tree(file_start))
{

    this->tokenizer = std::unique_ptr<Lexing::Tokenizer>(new Lexing::Tokenizer(input, 1));
}

void Parser::parse()
{
    if (tokenizer->isEmpty()) {
        return;
    }

    switch (tokenizer->peek()->token) {

    default:
        break;
    }
}

} // namespace Compiler::Parsing
