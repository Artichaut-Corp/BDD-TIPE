#include <iostream>
#include <ostream>
#include <string>
#include <utility>

#ifndef ERRORS_H

#define ERRORS_H

namespace Database::Errors {

typedef std::pair<int, std::string> ErrorCode;

const ErrorCode ERROR_UNIMPLEMENTED = std::pair(0, "Feature Unimplemented");
const ErrorCode ERROR_STRING_TYPE = std::pair(1, "Expected String Type");
const ErrorCode ERROR_INT_TYPE = std::pair(2, "Expected String Type");
const ErrorCode ERROR_ENDLINE = std::pair(3, "Expected Endline");
const ErrorCode ERROR_NO_LABEL = std::pair(4, "Using Undefined Label");
const ErrorCode ERROR_VAR = std::pair(5, "Using Undefined Variable");
const ErrorCode ERROR_UNEXPECTED_IDENTIFIER = std::pair(6, "Undeclared Indentifiers");
const ErrorCode ERROR_STRING_WITH_NO_DELIMETER = std::pair(7, "String Was Unterminated");
const ErrorCode ERROR_EXPECTED_KEYWORD = std::pair(8, "Expected a keyword");
const ErrorCode ERROR_EXPECTED_IDENTIFIER = std::pair(9, "Expected an identifier");
const ErrorCode ERROR_EXPECTED_ENDLINE = std::pair(9, "Expected an endline marker");
const ErrorCode ERROR_UNEXPECTED_SYMBOL = std::pair(10, "Unexpected symbol was encountred");
const ErrorCode ERROR_EXPECTED_SYMBOL = std::pair(11, "Expected a symbol");
const ErrorCode ERROR_EXPECTED_EXPRESSION = std::pair(12, "Expected an expression");
const ErrorCode ERROR_FILE_NOT_FOUND = std::pair(13, "File was not found");
const ErrorCode ERROR_WRONG_MEMORY_ACCESS = std::pair(14, "Unallowed access to memory");
const ErrorCode ERROR_UNEXPECTED_CALL_TO_FUNCTION = std::pair(15, "Unexpected call to function");
const ErrorCode ERROR_TABLE_DOES_NOT_EXIST = std::pair(16, "Tried to access a table that does not exist");
const ErrorCode ERROR_COLUMN_DOES_NOT_EXIST = std::pair(17, "Tried to access a column that does not exist");
const ErrorCode ERROR_UNKNOWN_ARGUMENT = std::pair(18, "Passed unrecognized argument");
const ErrorCode ERROR_UNGIVEN_ARGUMENT = std::pair(19, "Passed command without required argument");
const ErrorCode ERROR_UNRECOGNIZED_STMT = std::pair(20, "Used unrecognized statement");
const ErrorCode ERROR_NETWORK_FAILURE = std::pair(21, "Networking capabilities have failed");
const ErrorCode ERROR_TABLE_EMPTY = std::pair(22, "Requested an empty table");
const ErrorCode ERROR_UNKNOWN_TYPE_BINARYEXPR = std::pair(23, "Unknown type in the BinaryExpression Tree");
const ErrorCode ERROR_UNKNOWN_LOGICAL_OPERATOR = std::pair(24, "Unknown operator Type in Evaluation of a BinaryExpression");
const ErrorCode ERROR_UNKNOWN_TYPE = std::pair(25, "Found Unexpected Type");
const ErrorCode ERROR_UNMATCHED_ARG_NUMBER = std::pair(26, "Found different argument number");
const ErrorCode ERROR_FAILED_IO = std::pair(27, "Failed to proceed on file");

enum class ErrorType { CLIArgument,
    ParserError,
    SyntaxError,
    RuntimeError,
    SyntaxWarning };

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

    void printAllInfo(std::ostream& out) const
    {
        out << "ERROR "
            << "[[ " << m_ErrorCode.first << " ]] - " << m_ErrorCode.second << ":\n";
        out << m_Message << std::endl;
    }
}; // class Error

}; // namespace Compiler::Errors

#endif // !ERRORS_H
