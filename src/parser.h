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

namespace Database::Parsing {

using Statement = std::variant<SelectStmt*, InsertStmt*, UpdateStmt*, DeleteStmt*, Transaction*>;

class Parser {
private:
public:
    std::unique_ptr<Lexing::Tokenizer> m_Tokenizer;

    Parser(std::string input);

    ~Parser()
    {
    }

    [[nodiscard]] std::variant<Statement, Errors::Error> Parse();
};

} // namespace Compiler::Parsing

#endif // !PARSER_H
