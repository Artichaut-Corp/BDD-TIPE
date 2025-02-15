#pragma once

#include <iostream>
#include <string>
#include <utility>

namespace Compiler::Errors {

typedef std::pair<int, std::string> ErrorCode;

const ErrorCode ERROR_TESTING_ERROR = std::pair(0, "testing errors");
const ErrorCode ERROR_STRING_TYPE = std::pair(1, "Expected String Type");
const ErrorCode ERROR_INT_TYPE = std::pair(2, "Expected String Type");
const ErrorCode ERROR_ENDLINE = std::pair(3, "Unexpected endline");
const ErrorCode ERROR_NO_LABEL = std::pair(4, "Using Undefined Label");
const ErrorCode ERROR_VAR = std::pair(5, "Using Undefined Variable");
const ErrorCode ERROR_UNEXPECTED_IDENTIFIER = std::pair(6, "Undeclared Indentifiers");
const ErrorCode ERROR_STRING_WITH_NO_DELIMETER = std::pair(7, "String Was Unterminated");
const ErrorCode ERROR_EXPECTED_KEYWORD = std::pair(8, "Expected a keyword");
const ErrorCode ERROR_EXPECTED_IDENTIFIER = std::pair(9, "Expected an identifier");
const ErrorCode ERROR_EXPECTED_ENDLINE = std::pair(9, "Expected an endline marker");
const ErrorCode ERROR_UNEXPECTED_SYMBOL = std::pair(10, "Unexpected symbol was encountred");
const ErrorCode ERROR_EXPECTED_SYMBOL = std::pair(11, "Expected a symbol");
// Etc, I'll make them on need
const ErrorCode ERROR_TESTING = std::pair(12, "testing errors");
const ErrorCode ERROR_TESTIN = std::pair(13, "testing errors");
const ErrorCode ERROR_TESTI = std::pair(14, "testing errors");
const ErrorCode ERROR_TEST = std::pair(15, "testing errors");

enum class ErrorType { ParserError,
    SynxtaxError,
    RuntimeError,
    SynxtaxWarning };

class ParserInternalError : public std::exception {
public:
    std::string err()
    {
        return "Parser Encontered An Internal Error";
    }
};

class Error : ParserInternalError {
private:
    ErrorType m_Type;
    ErrorCode m_ErrorCode;
    std::string m_File;
    std::string m_Message;
    std::string m_Snipppet;
    int m_Line;
    int m_Col;

public:
    Error() = default;

    Error(ErrorType type, std::string message, int line, int col, ErrorCode code)
        : m_Type(type)
        , m_Message(message)
        , m_Line(line)
        , m_Col(col)
        , m_ErrorCode(code)
    {
    }

    void printLight()
    {
    }

    void printAllInfo()
    {
        try {
            std::cout << "ERROR "
                      << "[[ " << m_ErrorCode.first << " ]] - " << m_ErrorCode.second;
            std::cout << " at line " << m_Line << " and column " << m_Col << std::endl;
            std::cout << m_Message << std::endl;
        } catch (ParserInternalError) {
            throw ParserInternalError();
        }
    }
}; // class Error

}; // namespace Compiler::Errors
