#include "lexer/tokens.h"
#include "lexer/tokenizer.h"

#include <memory>
#include <string>

#ifndef PARSER_H
#define PARSER_H

namespace Compiler::Parsing {

class Parser {
private:
    std::unique_ptr<Lexing::Tokenizer> tokenizer;

public:
    Parser(Lexing::TokenType file_start, std::string input);
    ~Parser();

    void parse();
};
} // namespace Compiler::Parsing

#endif // !PARSER_H
