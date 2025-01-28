#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "tokens.h"

#include "../utils.h"
#include "../errors.h"

#ifndef LEXER_H
#define LEXER_H

namespace Compiler::Lexing {

class Lexer {
private:
    int line {};
    int curr_token {};

    std::string input_string;
    std::vector<Token> tokens;

public:
    Lexer(std::string str, int line);
    Lexer() = default;

    ~Lexer();

    size_t find_whitespace(const std::string str, size_t pos = 0);

    std::string cut_until_whitespace(std::string str);

    void setLine(const std::string& line);

    std::vector<std::string> split_string_on_whitespace(
        const std::string line, std::vector<std::string> res);

    std::variant<Token, Errors::Error> match_keyword(Token t, std::string str);

    std::variant<Token, Errors::Error> match_identifier(Token t, std::string str);

    std::optional<Errors::Error> identify_first();

    std::vector<Token> get_tokens();
};

} // namespace Compiler::Lexing

#endif // !LEXER_H
