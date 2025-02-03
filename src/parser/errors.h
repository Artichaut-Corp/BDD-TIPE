#pragma once

#include <iostream>
#include <string>
#include <utility>

namespace Compiler::Errors {

typedef std::pair<int, std::string> ErrorCode;

const ErrorCode ERROR_TESTING_ERROR = std::pair(0, "testing errors");
const ErrorCode ERROR_STRING_TYPE = std::pair(1, "Expected String Type");
const ErrorCode ERROR_INT_TYPE = std::pair(2, "Expected String Type");
const ErrorCode ERROR_NO_RETURN = std::pair(3, "Expected Return In Subroutine");
const ErrorCode ERROR_NO_LABEL = std::pair(4, "Using Undefined Label");
const ErrorCode ERROR_VAR = std::pair(5, "Using Undefined Variable");
const ErrorCode ERROR_UNEXPECTED_IDENTIFIER = std::pair(6, "Undeclared Indentifiers");
const ErrorCode ERROR_STRING_WITH_NO_DELIMETER = std::pair(7, "String Was Unterminated");
// Etc, I'll make them on need
const ErrorCode ERROR_TESTING_ERR = std::pair(8, "testing errors");
const ErrorCode ERROR_TESTING_ER = std::pair(9, "testing errors");
const ErrorCode ERROR_TESTING_E = std::pair(10, "testing errors");
const ErrorCode ERROR_TESTING_ = std::pair(11, "testing errors");
const ErrorCode ERROR_TESTING = std::pair(12, "testing errors");
const ErrorCode ERROR_TESTIN = std::pair(13, "testing errors");
const ErrorCode ERROR_TESTI = std::pair(14, "testing errors");
const ErrorCode ERROR_TEST = std::pair(15, "testing errors");

enum class ErrorType { ParserError,
    SynxtaxError,
    RuntimeError,
    SynxtaxWarning };

class InterpreterInternalError : public std::exception {
public:
    std::string err()
    {
        return "Interpreter Encontered An Internal Error";
    }
};

class Error : InterpreterInternalError {
private:
    ErrorType type;
    ErrorCode error_code;
    std::string file;
    std::string message;
    std::string snipppet;
    int line;
    int col;

public:
    Error() = default;

    Error(ErrorType type, std::string message, int line, int col, ErrorCode code)
    {
        this->type = type;
        this->message = message;
        this->line = line;
        this->col = col;
        this->error_code = code;
    }

    void print_light()
    {
    }

    void print_all_info()
    {
        try {
            std::cout << "ERROR "
                      << "[[ " << error_code.first << " ]] - " << error_code.second;
            std::cout << " at line " << line << " and column " << col << std::endl;
            std::cout << message << std::endl;
        } catch (InterpreterInternalError) {
            throw InterpreterInternalError();
        }
    }
}; // class Error

}; // namespace Compiler::Errors
