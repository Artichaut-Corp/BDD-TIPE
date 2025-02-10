#include "parser.h"

#include <memory>
#include <string>

#define UNIMPLEMENTED()                            \
    {                                              \
        throw std::runtime_error("UNIMPLEMENTED"); \
    }

namespace Compiler::Parsing {

Parser::Parser(Lexing::TokenType file_start, std::string input)
{

    this->m_Tokenizer = std::unique_ptr<Lexing::Tokenizer>(new Lexing::Tokenizer(input, 1));
}

Node<Statement>* Parser::Parse()
{
    if (m_Tokenizer->isEmpty()) {
        return nullptr;
    }

    switch (m_Tokenizer->peek()->m_Token) {

    case Lexing::DELETE_T: {
        DeleteStmt* r = DeleteStmt::ParseDelete();
    } break;
    default:
        UNIMPLEMENTED();
        break;
    }

    // A CHANGER ABSOLUMENT
    return nullptr;
}

} // namespace Compiler::Parsing
