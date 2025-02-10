#include "tokenizer.h"
#include "lexer.h"
#include "tokens.h"

#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace Compiler::Lexing {

std::unique_ptr<Token> Tokenizer::next(void)
{
    std::optional<Token> t = m_Tokens->get_first();

    m_Tokens->advance();

    return t.has_value() ? std::unique_ptr<Token>(&t.value()) : nullptr;
}

std::unique_ptr<Token> Tokenizer::peek(void) const
{

    std::optional<Token> t = m_Tokens->get_first();

    return t.has_value() ? std::unique_ptr<Token>(&t.value()) : nullptr;
}

bool Tokenizer::isEmpty()
{
    return this->m_Tokens == nullptr;
}

void Tokenizer::printAll() 
{
  m_Tokens->print_all();
}

void Tokenizer::setLine(const std::string& line)
{
    this->m_Lexer.setLine(line);
    this->m_Line += 1;
    this->m_Position = 0;
}

Tokenizer::Tokenizer(std::string line, int l_number)
    : m_Lexer(line, l_number)
{
    auto tok = m_Lexer.getTokens();

    auto list = Utils::LinkedList<Token>();

    tok->print_all();
    list.print_all();

    while (!tok->is_empty()) {
        // Not safe, should check what's inside

        if (tok->get_first().has_value())
            list.append(tok->get_first().value());

        tok->advance();
    }

    this->m_Tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);
}

Tokenizer::~Tokenizer()
{
    std::cout << "----- [DEBUG] -----\n\n";
    printAll();
    std::cout << "----- [DEBUG] -----\n";
}

} // namespace Compiler::Lexing
