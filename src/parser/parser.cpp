#include "parser.h"
#include "errors.h"

#include <memory>
#include <string>

#define UNIMPLEMENTED()                            \
    {                                              \
        throw std::runtime_error("UNIMPLEMENTED"); \
    }

namespace Compiler::Parsing {

Parser::Parser(std::string input)
{
    this->m_Tokenizer = std::unique_ptr<Lexing::Tokenizer>(new Lexing::Tokenizer(input, 1));
}

std::variant<Statement*, Errors::Error> Parser::Parse()
{

    std::variant<Statement*, Errors::Error> node;

    if (m_Tokenizer->isEmpty()) {
        node = Errors::Error();
        return node;
    }

    m_Tokenizer->printAll();

    try {
        switch (m_Tokenizer->peek().m_Token) {

        case Lexing::DELETE_T: {
            Statement r = DeleteStmt::ParseDelete(m_Tokenizer.get());
        } break;
        case Lexing::INSERT_T: {
            Statement r = InsertStmt::ParseInsert(m_Tokenizer.get());
        } break;
        case Lexing::UPDATE_T: {
            Statement r = UpdateStmt::ParseUpdate(m_Tokenizer.get());
        } break;
        case Lexing::SELECT_T: {
            Statement r = SelectStmt::ParseSelect(m_Tokenizer.get());
        } break;
        default:
            UNIMPLEMENTED();
            break;
        }
    } catch (Errors::Error& e) {
        node = e;
    }

    return node;
}

} // namespace Compiler::Parsing
