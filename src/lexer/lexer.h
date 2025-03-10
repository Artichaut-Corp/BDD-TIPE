#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "tokens.h"

#include "../errors.h"
#include "../utils.h"

#ifndef LEXER_H
#define LEXER_H

namespace Database::Lexing {

class Lexer {

private:
    int m_Line {};
    int m_CurrToken {};

    std::string m_InputString;

    std::unique_ptr<Utils::LinkedList<Token>> m_Tokens;

    size_t findWhitespace(const std::string str, size_t pos = 0);

    std::string cutUntilWhitespace(std::string str);

    std::vector<std::string> splitStringOnWhitespace(
        const std::string line, std::vector<std::string> res);

    std::variant<Token, Errors::Error> matchKeyword(Token t, std::string str);

    std::variant<Token, Errors::Error> matchIdentifier(Token t, std::string str);

    std::optional<Errors::Error> identifyFirst();

public:
    Lexer(std::string str, int line);

    Lexer(const Lexer& other);

    Lexer& operator=(const Lexer& rhs)
    {

        auto list = Utils::LinkedList<Token>();

        while (!rhs.m_Tokens->is_empty()) {
            // Not safe, should check what's inside
            list.append(rhs.m_Tokens->get_first().value());
        }

        this->m_Tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);

        return *this;
    }

    ~Lexer();

    void setLine(const std::string& line);

    std::unique_ptr<Utils::LinkedList<Token>> getTokens();
};

} // namespace Compiler::Lexing

#endif // !LEXER_H
