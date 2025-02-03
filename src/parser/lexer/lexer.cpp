#include "lexer.h"

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <variant>

#include "tokens.h"

#include "../errors.h"
#include "../utils.h"

namespace Compiler::Lexing {

Lexer::Lexer(std::string str, int line)
    : input_string(str)
    , line(line)
{

    // auto list = Utils::LinkedList<Token>();

    this->curr_token = 0;
    this->tokens = std::unique_ptr<Utils::LinkedList<Token>>(new Utils::LinkedList<Token>);
}

Lexer::Lexer(const Lexer& other)
    : input_string(other.input_string)
    , line(other.line)
    , curr_token(other.curr_token)
{
    auto list = Utils::LinkedList<Token>();

    while (!other.tokens->is_empty()) {

        if (other.tokens->get_first().has_value())
            list.append(other.tokens->get_first().value());
    }

    this->tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);
}

Lexer::~Lexer() { return; }

std::unique_ptr<Utils::LinkedList<Token>> Lexer::get_tokens()
{
    std::optional<Errors::Error> err;

    for (;;) {

        if (this->curr_token >= this->input_string.length()) {
            return std::move(this->tokens);
        }

        err = identify_first();

        if (err.has_value()) {
            err->print_all_info();

            Utils::LinkedList<Token> l;

            return std::unique_ptr<Utils::LinkedList<Token>>(&l);
        }
    }

    return std::move(this->tokens);
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
    //
    case 39: // c'est '
    {
        std::string str = "";
        std::advance(it, 1);
        while (*it != 32) {
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
        t.create_token(STRING_LITT_T, str);
    } break;
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
        t.create_token(STRING_LITT_T, str);
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
    case '.':
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
        t.create_token(NUM_LITT_T, num);
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
    this->tokens->append(t);

    // Cut until the next token
    this->input_string = this->input_string.substr(this->curr_token);

    // Reset Counter
    //curr_token = 0;

    // Return no syntax err
    return err;
}

std::variant<Token, Errors::Error> Lexer::match_keyword(Token t,
    std::string str)
{
    std::variant<Token, Errors::Error> variant;
    std::string keywd = Utils::to_uppercase(str);
    if (keywd == "SELECT")
        t.create_token(SELECT_T);
    else if (keywd == "FROM")
        t.create_token(FROM_T);
    else if (keywd == "WHERE")
        t.create_token(WHERE_T);
    else if (keywd == "AS")
        t.create_token(AS_T);
    else if (keywd == "GROUP")
        t.create_token(GROUP_T);
    else if (keywd == "BY")
        t.create_token(BY_T);
    else if (keywd == "HAVING")
        t.create_token(HAVING_T);
    else if (keywd == "ORDER")
        t.create_token(ORDER_T);
    else if (keywd == "JOIN")
        t.create_token(JOIN_T);
    else if (keywd == "ON")
        t.create_token(ON_T);
    else if (keywd == "LEFT")
        t.create_token(LEFT_T);
    else if (keywd == "RIGHT")
        t.create_token(RIGHT_T);
    else if (keywd == "INNER")
        t.create_token(INNER_T);
    else if (keywd == "OUTER")
        t.create_token(OUTER_T);
    else if (keywd == "SET")
        t.create_token(SET_T);
    else if (keywd == "INSERT")
        t.create_token(INSERT_T);
    else if (keywd == "INTO")
        t.create_token(INTO_T);
    else if (keywd == "DELETE")
        t.create_token(DELETE_T);
    else if (keywd == "CREATE")
        t.create_token(CREATE_T);
    else if (keywd == "DROP")
        t.create_token(DROP_T);
    else if (keywd == "TABLE")
        t.create_token(TABLE_T);
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
    /*
    if (*it == '$') {
        t.create_token(STR_VAR_T, str);
    } else if (*it == '%') {
        t.create_token(NUM_VAR_T, str);
    } else {
        // return err
        // variant = errors::Error(errors::ErrorType::LexerError, "Identifiers
        // Doesnt Have A Closing Mark", line, errors::ERROR_UNEXPECTED_IDENTIFIER);
        // return variant;
    }
    */
    t.create_token(VAR_NAME_T, str);

    variant = t;
    return variant;
}

void Lexer::setLine(const std::string& line)
{
    this->input_string = line;
    this->line = 0;
    this->curr_token = 0;

    auto list = Utils::LinkedList<Token>();

    this->tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);
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
