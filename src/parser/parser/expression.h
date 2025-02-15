#include <memory>
#include <optional>
#include <string>

#include "../lexer/tokenizer.h"

#ifndef EXPRESSION_H

#define EXPRESSION_H

namespace Compiler::Parsing {

enum class LogicalOperator { EQ,
    GT,
    LT,
    GTE,
    LTE,
    NE,
    AND,
    OR,
    NOT };

std::variant<LogicalOperator, Errors::Error> ParseLogicalOperator(Lexing::Tokenizer* t);

enum class ColumnType { NULL_C,
    INTEGER_C,
    REAL_C,
    TEXT_C };

class Expr { };

template <typename T>
class LitteralValue : public Expr {
    ColumnType m_Type;

    T m_Data;

public:
    LitteralValue(ColumnType type, T data)
        : m_Type(type)
        , m_Data(data)
    {
    }
};

class SchemaName : public Expr {
    std::string m_Name;

    std::optional<std::string> m_TableName;

    std::optional<std::string> m_ColumnName;

public:
    SchemaName(std::string name)
        : m_Name(name)
        , m_TableName(std::nullopt)
        , m_ColumnName(std::nullopt)

    {
    }

    SchemaName(std::string name, std::string table_name)
        : m_Name(name)
        , m_TableName(table_name)
        , m_ColumnName(std::nullopt)

    {
    }

    SchemaName(std::string name, std::string table_name, std::string column_name)
        : m_Name(name)
        , m_TableName(table_name)
        , m_ColumnName(column_name)

    {
    }

    static Expr* ParseSchemaName(Lexing::Tokenizer* t);
};

class TableName : public Expr {
    std::string m_TableName;

    std::optional<std::string> m_ColumnName;

public:
    // One constructor for only a table name
    TableName(std::string name)
        : m_TableName(name)
        , m_ColumnName(std::nullopt)
    {
    }

    // One constructor for schema AND table names
    TableName(std::string name, std::string column_name)
        : m_TableName(name)
        , m_ColumnName(column_name)
    {
    }

    static Expr* ParseTableName(Lexing::Tokenizer* t);
};

class ColumnName : public Expr {
    std::string m_Name;

public:
    ColumnName() = default;
    ColumnName(std::string name)
        : m_Name(name)
    {
    }

    static Expr* ParseColumnName(Lexing::Tokenizer* t);
};

class BinaryExpression : Expr {
    // Cas Récursifs
    std::optional<std::unique_ptr<BinaryExpression>> m_Lhs;
    std::optional<std::unique_ptr<BinaryExpression>> m_Rhs;

    // Cas de base
    std::optional<std::unique_ptr<Expr>> m_ExprLeft;
    std::optional<std::unique_ptr<Expr>> m_ExprRight;

    // Opérateur
    std::optional<LogicalOperator> m_Op;

public:
    // Cas de base, expression seulement
    BinaryExpression(Expr* expr)
        : m_Op(std::nullopt)
        , m_ExprLeft(std::nullopt)
        , m_ExprRight(expr)
        , m_Lhs(std::nullopt)
        , m_Rhs(std::nullopt)
    {
    }

    // Cas récursif simple, probablement pas nécessaire
    BinaryExpression(Expr* exprLeft, LogicalOperator op, Expr* exprRight)
        : m_Op(op)
        , m_ExprLeft(exprLeft)
        , m_ExprRight(exprRight)
        , m_Lhs(std::nullopt)
        , m_Rhs(std::nullopt)
    {
    }

    // Un ou deux cas récursifs: a = b OR b = c
    BinaryExpression(Expr* exprLeft, BinaryExpression* lhs, LogicalOperator op,
        Expr* exprRight,
        BinaryExpression* rhs)
        : m_Op(op)
        , m_ExprLeft(exprLeft)
        , m_ExprRight(exprRight)
        , m_Lhs(lhs)
        , m_Rhs(rhs)
    {
    }

    static BinaryExpression* ParseBinaryExpression(Lexing::Tokenizer* t);
};

} // namespace parsing

#endif // !EXPRESSION_H
