#include "lexer/tokenizer.h"
#include "lexer/tokens.h"

#include "parser/ddl.h"
#include "parser/dml.h"
#include "parser/expression.h"
#include "parser/node.h"

#include <memory>
#include <string>
#include <variant>

#ifndef PARSER_H
#define PARSER_H

namespace Compiler::Parsing {

using Statement = std::variant<SelectStmt, InsertStmt, UpdateStmt, DeleteStmt>;

class Parser {
private:
    std::unique_ptr<Lexing::Tokenizer> m_Tokenizer;

public:
    Parser(Lexing::TokenType file_start, std::string input);
    ~Parser();

    Node<Statement>* Parse();
};
} // namespace Compiler::Parsing

#endif // !PARSER_H
