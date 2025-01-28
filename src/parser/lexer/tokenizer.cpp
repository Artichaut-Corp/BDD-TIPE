#include "tokenizer.h"
#include "lexer.h"
#include "tokens.h"

#include <iostream>
#include <string>
#include <vector>

namespace Compiler::Lexing {

Token Tokenizer::next(void)
{
    int pos = (position >= tokens.size()) ? 0 : position++;
    return tokens[pos];
}

Token Tokenizer::peek(void) const
{
    return tokens[position];
}

void Tokenizer::printAll(void) const
{
    std::cout << "[ ";
    for (auto t : tokens) {
      t.print();
    }
    std::cout << "]\n";
}

void Tokenizer::setLine(const std::string& line)
{
    this->l.setLine(line);
    this->line += 1;
    this->position = 0;
}

Tokenizer::Tokenizer(std::string line, int l_number)
{
    this->l = Lexer(line, l_number);

    std::vector<Token> tokens = l.get_tokens();

    if (!tokens.empty()) {
        this->tokens = tokens;
    }
}

Tokenizer::~Tokenizer()
{
    std::cout << "[DEBUG]\n";
    printAll();
    std::cout << "[DEBUG]\n";
}

} // namespace Compiler::Lexing
