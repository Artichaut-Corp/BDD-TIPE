#include <string>
#include <memory>

#include "lexer.h"
#include "tokens.h"

#ifndef TOKENIZER_H
#define TOKENIZER_H

namespace Compiler::Lexing {

// Interface accessing tokens
class Tokenizer {
private:
    Lexer m_Lexer;

    int m_Position = 0;
    int m_Line;
    std::unique_ptr<Utils::LinkedList<Token>> m_Tokens = nullptr;

public:
    std::unique_ptr<Token>  next(void);
    std::unique_ptr<Token>  peek(void) const;

    bool isEmpty();

    void printAll(void);

    void setLine(const std::string& line);

    Tokenizer(std::string line, int l_number);

    ~Tokenizer();
};

} // namespace Compiler::Lexing

#endif // !TOKENIZER_H
