#include "parser.h"
#include "errors.h"

#include <memory>
#include <string>

namespace Database::Parsing {

Parser::Parser(std::string input)
{
    this->m_Tokenizer = std::unique_ptr<Lexing::Tokenizer>(new Lexing::Tokenizer(input, 1));
}

std::variant<Statement, Errors::Error> Parser::Parse()
{

    std::variant<Statement, Errors::Error> node;

    if (m_Tokenizer->isEmpty()) {
        node = Errors::Error();
        return node;
    }

    try {
        switch (m_Tokenizer->peek().m_Token) {

        case Lexing::DELETE_T: {
            return DeleteStmt::ParseDelete(m_Tokenizer.get());
        } break;
        case Lexing::INSERT_T: {
            return InsertStmt::ParseInsert(m_Tokenizer.get());
        } break;
        case Lexing::UPDATE_T: {
            return UpdateStmt::ParseUpdate(m_Tokenizer.get());
        } break;
        case Lexing::SELECT_T: {
            return SelectStmt::ParseSelect(m_Tokenizer.get());
        } break;
        case Lexing::TRANSACTION_T: {
            return Transaction::ParseTransaction(m_Tokenizer.get());
        }
        default:
            node = Errors::Error(Errors::ErrorType::SyntaxError, "Expected SQL Statement", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
            break;
        }
    } catch (Errors::Error& e) {
        node = e;
    }

    return node;
}

} // namespace Compiler::Parsing
