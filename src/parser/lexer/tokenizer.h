#include <string>
#include <vector>

#include "lexer.h"
#include "tokens.h"

#ifndef TOKENIZER_H
#define TOKENIZER_H

namespace Compiler::Lexing {

// Interface accessing tokens
class Tokenizer {
private:
    Lexer l;

    int position = 0;
    int line;
    std::vector<Token> tokens;

public:
    Token next(void);
    Token peek(void) const;

    void printAll(void) const;

    void setLine(const std::string& line);

    Tokenizer() = default;
    Tokenizer(std::string line, int l_number);

    ~Tokenizer();
};

} // namespace Compiler::Lexing

#endif // !TOKENIZER_H
