#include "lexer/tokenizer.h"
#include "lexer/tokens.h"

#include "parser/ddl.h"
#include "parser/dml.h"
#include "parser/expression.h"
#include "parser/node.h"

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
