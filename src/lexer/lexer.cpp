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

namespace Database::Lexing {

Lexer::Lexer(std::string str, int line)
    : m_InputString(str)
    , m_Line(line)
    , m_CurrToken(0)
{
    this->m_Tokens = std::unique_ptr<Utils::LinkedList<Token>>(new Utils::LinkedList<Token>);
}

Lexer::Lexer(const Lexer& other)
    : m_InputString(other.m_InputString)
    , m_Line(other.m_Line)
    , m_CurrToken(other.m_CurrToken)
{
    auto list = Utils::LinkedList<Token>();

    while (!other.m_Tokens->is_empty()) {

        if (other.m_Tokens->get_first().has_value())
            list.append(other.m_Tokens->get_first().value());
    }

    this->m_Tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);
}

Lexer::~Lexer() { return; }

std::unique_ptr<Utils::LinkedList<Token>> Lexer::getTokens()
{
    std::optional<Errors::Error> err;

    for (;;) {
        if (this->m_CurrToken >= this->m_InputString.length()) {
            return std::move(this->m_Tokens);
        }

        err = identifyFirst();

        if (err.has_value()) {
            err->printAllInfo();

            Utils::LinkedList<Token> l;

            return std::unique_ptr<Utils::LinkedList<Token>>(&l);
        }
    }

    return std::move(this->m_Tokens);
}

// Given a string, returns the first identifier and adds it to the Object
// property
std::optional<Errors::Error> Lexer::identifyFirst()
{
    using namespace Errors;

    Token t {};
    std::optional<Error> err = std::nullopt;
    auto it = std::begin(this->m_InputString); // it != std::end(input); it++) {

    // Ignore all spaces
    while (isspace(*it)) {
        std::advance(it, 1);
        m_CurrToken++;
    }

    // Then identify the first character
    switch (*it) {
    // 1st to 31th are controls characters (Maybe EOF and tab could be used);
    // ignored and handled in default case EOF and Newline are meant to stop
    // parsing the line or the file
    case 2:
        t.createToken(START_T);
        break;
    case 3:
        t.createToken(EOF_T);
        break;
    case '\n':
        t.createToken(ENDL_T);
        break;
    // 32th is space, already ignored
    // Read until the next '"' and identify as a string
    //
    case 39: // c'est '
    {
        std::string str = "";
        std::advance(it, 1);
        while (*it != 39) {
            if (m_CurrToken > m_InputString.length()) {
                return std::make_optional(Error(ErrorType::SyntaxError, "", m_Line,
                    m_CurrToken,
                    ERROR_STRING_WITH_NO_DELIMETER));
            }

            str.push_back(*it);

            std::advance(it, 1);
            m_CurrToken++;
        }

        // Skip 2 (last ' included)
        m_CurrToken += 2;
        t.createToken(STRING_LITT_T, str);
    } break;
    case '(':
        t.createToken(LPAREN_T);
        m_CurrToken++;
        break;
    case ')':
        t.createToken(RPAREN_T);
        m_CurrToken++;
        break;
    case '"': {
        std::string str = "";
        std::advance(it, 1);
        while (*it != '"') {
            if (m_CurrToken > m_InputString.length()) {
                return std::make_optional(Error(ErrorType::SyntaxError, "", m_Line,
                    m_CurrToken,
                    ERROR_STRING_WITH_NO_DELIMETER));
            }

            str.push_back(*it);

            std::advance(it, 1);
            m_CurrToken++;
        }

        // Skip 2 (last " included)
        m_CurrToken += 2;
        t.createToken(STRING_LITT_T, str);
    } break;
    // Operators, either mathematical or logical
    case '*':
    case '+':
    case '-':
        t.createToken(MATH_OP_T, std::string { *it });
        m_CurrToken++;
        break;
    case '<':
    case '>':
    case '=':
    case '/':
    case '!':
    case '&':
    case '|':

        t.createToken(EQ_OP_T, std::string { *it });
        m_CurrToken++;
        break;
    // Punctuation
    case ',':
        t.createToken(COMMA_T);
        m_CurrToken++;
        break;
    case ';':
        t.createToken(SEMI_COLON_T);
        m_CurrToken++;
        break;
    case '.':
        t.createToken(DOT_T);
        m_CurrToken++;
        break;
    // Numbers from 0 to 9. FLOATS NOT IMPLEMENTED
    case 49 ... 57: {
        std::string num = "";
        do {
            num.push_back(*it);
            std::advance(it, 1);
            m_CurrToken++;

        } while (
            std::isdigit(*it)); // OR is . if I implement floating point numbers
        t.createToken(NUM_LITT_T, num);
    } break;
    // Letters, ignoring character's case
    case 65 ... 90:
    case 97 ... 122: {
        std::string keyword;
        while (std::isalpha(*it) || *it == '%' || *it == '$' || *it == ':') {
            keyword.push_back(*it);
            std::advance(it, 1);
            m_CurrToken++;
        }

        std::variant<Token, Error> token_or_err = matchKeyword(t, keyword);

        if (std::holds_alternative<Error>(token_or_err)) {
            return std::get<Error>(token_or_err);
        }

        t = std::get<Token>(token_or_err);
    } break;
    // In default case just add the token
    default:
        err = std::make_optional(Error(ErrorType::SyntaxError,
            "Unexpected Character Found", m_Line,
            m_CurrToken, ERROR_UNEXPECTED_IDENTIFIER));
    } // End Switch

    // Store the token
    this->m_Tokens->append(t);

    // Cut until the next token
    this->m_InputString = this->m_InputString.substr(this->m_CurrToken);

    // Reset Counter
    m_CurrToken = 0;

    // Return no syntax err
    return err;
}

std::variant<Token, Errors::Error> Lexer::matchKeyword(Token t,
    std::string str)
{
    std::variant<Token, Errors::Error> variant;
    std::string keywd = Utils::to_uppercase(str);
    if (keywd == "SELECT")
        t.createToken(SELECT_T);
    else if (keywd == "UPDATE")
        t.createToken(UPDATE_T);
    else if (keywd == "INSERT")
        t.createToken(INSERT_T);
    else if (keywd == "DELETE")
        t.createToken(DELETE_T);
    else if (keywd == "FROM")
        t.createToken(FROM_T);
    else if (keywd == "SET")
        t.createToken(SET_T);
    else if (keywd == "INTO")
        t.createToken(INTO_T);
    else if (keywd == "WHERE")
        t.createToken(WHERE_T);
    else if (keywd == "GROUP")
        t.createToken(GROUP_T);
    else if (keywd == "BY")
        t.createToken(BY_T);
    else if (keywd == "HAVING")
        t.createToken(HAVING_T);
    else if (keywd == "ORDER")
        t.createToken(ORDER_T);
    else if (keywd == "ASC")
        t.createToken(ASC_T);
    else if (keywd == "DESC")
        t.createToken(DESC_T);
    else if (keywd == "LIMIT")
        t.createToken(LIMIT_T);
    else if (keywd == "OFFSET")
        t.createToken(OFFSET_T);
    else if (keywd == "VALUES")
        t.createToken(VALUES_T);
    else if (keywd == "DEFAULT")
        t.createToken(DEFAULT_T);
    else if (keywd == "JOIN")
        t.createToken(JOIN_T);
    else if (keywd == "ON")
        t.createToken(ON_T);
    else if (keywd == "LEFT")
        t.createToken(LEFT_T);
    else if (keywd == "RIGHT")
        t.createToken(RIGHT_T);
    else if (keywd == "INNER")
        t.createToken(INNER_T);
    else if (keywd == "OUTER")
        t.createToken(OUTER_T);
    else if (keywd == "FULL")
        t.createToken(FULL_T);
    else if (keywd == "CROSS")
        t.createToken(CROSS_T);
    else if (keywd == "CREATE")
        t.createToken(CREATE_T);
    else if (keywd == "RENAME")
        t.createToken(RENAME_T);
    else if (keywd == "DROP")
        t.createToken(DROP_T);
    else if (keywd == "ALTER")
        t.createToken(ALTER_T);
    else if (keywd == "TABLE")
        t.createToken(TABLE_T);
    else if (keywd == "DATABASE")
        t.createToken(DATABASE_T);
    else if (keywd == "UNIQUE")
        t.createToken(UNIQUE_T);
    else if (keywd == "NOT")
        t.createToken(NOT_T);
    else if (keywd == "IS")
        t.createToken(IS_T);
    else if (keywd == "DISTINCT")
        t.createToken(DISTINCT_T);
    else if (keywd == "AND")
        t.createToken(AND_T);
    else if (keywd == "OR")
        t.createToken(OR_T);
    else if (keywd == "NULL")
        t.createToken(NULL_T);
    else if (keywd == "FOREIGN")
        t.createToken(FOREIGN_T);
    else if (keywd == "PRIMARY")
        t.createToken(PRIMARY_T);
    else if (keywd == "KEY")
        t.createToken(KEY_T);
    else {
        return matchIdentifier(t, str);
    }

    variant = t;
    return variant;
}

std::variant<Token, Errors::Error> Lexer::matchIdentifier(Token t, std::string str)
{
    std::variant<Token, Errors::Error> variant;

    const std::string& s = Utils::to_uppercase(str);

    if (s == "AVG" || s == "COUNT" || s == "MAX" || s == "MIN" || s == "SUM") {
        t.createToken(AGGR_FUNC_T, s);
    } else {
        t.createToken(VAR_NAME_T, str);
    }

    variant = t;
    return variant;
}

void Lexer::setLine(const std::string& line)
{
    this->m_InputString = line;
    this->m_Line = 0;
    this->m_CurrToken = 0;

    auto list = Utils::LinkedList<Token>();

    this->m_Tokens = std::unique_ptr<Utils::LinkedList<Token>>(&list);
}

inline std::string Lexer::cutUntilWhitespace(std::string str)
{
    str.erase(str.begin(), str.begin() + findWhitespace(str));
    return str;
}

// Split a string into a vec on each whitespace
std::vector<std::string> Lexer::splitStringOnWhitespace(
    const std::string line, std::vector<std::string> res)
{
    size_t first_ws = findWhitespace(Utils::trim(line));

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

    return splitStringOnWhitespace(cutUntilWhitespace(line), res);
}

// Returns the first whitespace-like char in a string until a given position.
// If nothing is found, return string::npos instead
size_t Lexer::findWhitespace(std::string str, size_t pos)
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
