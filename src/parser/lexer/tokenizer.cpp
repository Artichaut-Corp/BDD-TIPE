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
    std::optional<Token> t = tokens->get_first();

    tokens->advance();

    return t.has_value() ? std::unique_ptr<Token>(&t.value()) : nullptr;
}

std::unique_ptr<Token> Tokenizer::peek(void) const
{

    std::optional<Token> t = tokens->get_first();

    return t.has_value() ? std::unique_ptr<Token>(&t.value()) : nullptr;
}

bool Tokenizer::isEmpty()
{
    return this->tokens == nullptr;
}

void Tokenizer::printAll() 
{
  tokens->print_all();
}

void Tokenizer::setLine(const std::string& line)
{
    this->l.setLine(line);
    this->line += 1;
    this->position = 0;
}

Tokenizer::Tokenizer(std::string line, int l_number)
    : l(line, l_number)
{
    auto tok = l.get_tokens();

    auto list = Utils::LinkedList<Token>();

    tok->print_all();
    list.print_all();

    while (!tok->is_empty()) {
        // Not safe, should check what's inside

        if (tok->get_first().has_value())
            list.append(tok->get_first().value());

        tok->advance();
    }

    this->tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);
}

Tokenizer::~Tokenizer()
{
    std::cout << "----- [DEBUG] -----\n\n";
    printAll();
    std::cout << "----- [DEBUG] -----\n";
}

} // namespace Compiler::Lexing
