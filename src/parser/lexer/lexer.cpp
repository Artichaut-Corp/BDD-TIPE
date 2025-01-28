#include "lexer.h"

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <optional>
#include <ostream>
#include <string>
#include <variant>

#include "tokens.h"

#include "../errors.h"
#include "../utils.h"

namespace Compiler::Lexing {

// Beaucoup à changer ici aussi

Lexer::Lexer(std::string str, int line)
{
    this->input_string = str;
    this->line = line;
}

Lexer::~Lexer() { return; }

std::vector<Token> Lexer::get_tokens()
{
    std::optional<Errors::Error> err;

    for (;;) {
        if (this->curr_token >= this->input_string.length()) {
            return this->tokens;
        }

        err = identify_first();

        if (err.has_value()) {
            err->print_all_info();
            std::vector<Token> v;
            return v;
        }
    }
    
    return this->tokens;
}

// Given a string, returns the first identifier and adds it to the Object
// property
std::optional<Errors::Error> Lexer::identify_first()
{
    using namespace Errors;

    Token t {};
    std::optional<Error> err = std::nullopt;
    auto it = std::begin(this->input_string); // it != std::end(input); it++) {

    // Ignore all spaces
    while (isspace(*it)) {
        std::advance(it, 1);
        curr_token++;
    }

    // Then identify the first character
    switch (*it) {
    // 1st to 31th are controls characters (Maybe EOF and tab could be used);
    // ignored and handled in default case EOF and Newline are meant to stop
    // parsing the line or the file
    case 2:
        t.create_token(START_T);
        break;
    case 3:
        t.create_token(EOF_T);
        break;
    case '\n':
        t.create_token(ENDL_T);
        break;
    // 32th is space, already ignored
    // Read until the next '"' and identify as a string
    case '"': {
        std::string str = "";
        std::advance(it, 1);
        while (*it != '"') {
            if (curr_token > input_string.length()) {
                return std::make_optional(Error(ErrorType::SynxtaxError, "", line,
                    curr_token,
                    ERROR_STRING_WITH_NO_DELIMETER));
            }

            str.push_back(*it);

            std::advance(it, 1);
            curr_token++;
        }

        // Skip 2 (last " included)
        curr_token += 2;
        t.create_token(STRING_T, str);
    } break;
    // Operators, either mathematical or logical
    case '*':
    case '+':
    case '-':
    case '<':
    case '>':
    case '=':
    case '/':
        t.create_token(EQ_OP_T, std::to_string(*it));
        curr_token++;
        break;
    // Punctuation
    case ',':
    case ';':
        t.create_token(PUNCT_T, std::to_string(*it));
        curr_token++;
        break;
    // Numbers from 0 to 9
    case 49 ... 57: {
        std::string num = "";
        do {
            num.push_back(*it);
            std::advance(it, 1);
            curr_token++;

        } while (
            std::isdigit(*it)); // OR is . if I implement floating point numbers
        t.create_token(NUMBER_T, num);
    } break;
    // Letters, ignoring character's case
    case 65 ... 90:
    case 97 ... 122: {
        std::string keyword;
        while (std::isalpha(*it) || *it == '%' || *it == '$' || *it == ':') {
            keyword.push_back(*it);
            std::advance(it, 1);
            curr_token++;
        }

        std::variant<Token, Error> token_or_err = match_keyword(t, keyword);

        if (std::holds_alternative<Error>(token_or_err)) {
            return std::get<Error>(token_or_err);
        }

        t = std::get<Token>(token_or_err);
    } break;
    // In default case just add the token
    default:
        err = std::make_optional(Error(ErrorType::SynxtaxError,
            "Unexpected Character Found", line,
            curr_token, ERROR_UNEXPECTED_IDENTIFIER));
    } // End Switch

    // Store the token
    this->tokens.push_back(t);

    // Cut until the next token
    this->input_string = this->input_string.substr(this->curr_token);

    // Reset Counter
    curr_token = 0;

    // Return no syntax err
    return err;
}

std::variant<Token, Errors::Error> Lexer::match_keyword(Token t,
    std::string str)
{
    std::variant<Token, Errors::Error> variant;
    std::string keywd = Utils::to_uppercase(str);
    if (keywd == "LET")
        t.create_token(LET_T);
    else if (keywd == "DATA")
        t.create_token(DATA_T);
    else if (keywd == "READ")
        t.create_token(READ_T);
    else if (keywd == "RESTORE")
        t.create_token(RESTORE_T);
    else if (keywd == "IF")
        t.create_token(IF_T);
    else if (keywd == "THEN")
        t.create_token(THEN_T);
    else if (keywd == "ELSE")
        t.create_token(ELSE_T);
    else if (keywd == "FOR")
        t.create_token(FOR_T);
    else if (keywd == "TO")
        t.create_token(TO_T);
    else if (keywd == "STEP")
        t.create_token(THEN_T);
    else if (keywd == "WHILE")
        t.create_token(WHILE_T);
    else if (keywd == "WEND")
        t.create_token(WEND_T);
    else if (keywd == "REPEAT")
        t.create_token(REPEAT_T);
    else if (keywd == "UNTIL")
        t.create_token(UNTIL_T);
    else if (keywd == "DO")
        t.create_token(DO_T);
    else if (keywd == "LOOP")
        t.create_token(LOOP_T);
    else if (keywd == "ON")
        t.create_token(ON_T);
    else if (keywd == "GOTO")
        t.create_token(GOTO_T);
    else if (keywd == "GOSUB")
        t.create_token(GOSUB_T);
    else if (keywd == "RETURN")
        t.create_token(RETURN_T);
    else if (keywd == "LIST")
        t.create_token(LIST_T);
    else if (keywd == "PRINT")
        t.create_token(PRINT_T);
    else {
        return match_identifier(t, str);
    }

    variant = t;
    return variant;
}

std::variant<Token, Errors::Error> Lexer::match_identifier(Token t, std::string str)
{
    auto it = str.end();

    std::variant<Token, Errors::Error> variant;

    if (*it == '$') {
        t.create_token(STR_VAR_T, str);
    } else if (*it == '%') {
        t.create_token(NUM_VAR_T, str);
    } else if (*it == ':') {
        t.create_token(LABEL_T, str);
    } else {
        // return err
        // variant = errors::Error(errors::ErrorType::LexerError, "Identifiers
        // Doesnt Have A Closing Mark", line, errors::ERROR_UNEXPECTED_IDENTIFIER);
        // return variant;
        t.create_token(VAR_DECL_T, str);
    }

    variant = t;
    return variant;
}

void Lexer::setLine(const std::string& line)
{
    this->input_string = line;
    this->line = 0;
    this->curr_token = 0;
    this->tokens.clear();
}

inline std::string Lexer::cut_until_whitespace(std::string str)
{
    str.erase(str.begin(), str.begin() + find_whitespace(str));
    return str;
}

// Split a string into a vec on each whitespace
std::vector<std::string> Lexer::split_string_on_whitespace(
    const std::string line, std::vector<std::string> res)
{
    size_t first_ws = find_whitespace(Utils::trim(line));

    std::cout << "\"" << line << "\""
              << "\n";
    std::cout << first_ws << "\n";
    if (first_ws == std::string::npos) {
        std::cout << "[ ";
        for (std::string token : res) {
            std::cout << token << ", ";
        }
        std::cout << "]" << std::endl;
        return res;
    }

    std::string token;

    token = Utils::trim(line.substr(0, first_ws));
    std::cout << token << "\n";
    res.push_back(token);

    return split_string_on_whitespace(cut_until_whitespace(line), res);
}

// Returns the first whitespace-like char in a string until a given position.
// If nothing is found, return string::npos instead
size_t Lexer::find_whitespace(std::string str, size_t pos)
{
    size_t i = 1;
    for (char c : str) {
        // Check if the current char is whitespace like
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
            // If so, return the index if it
            return i;
        }

        // Then check if we got as far as permitted by the parameter
        if (pos > i)
            break;
        // Else we can increment position and loop
        i++;
        continue;
    }
    // If nothing had been found, return npos
    return std::string::npos;
}

} // namespace Compiler::Lexing
