#include <optional>
#include <string>
#include "memory"

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

public:
    SchemaName(std::string name)
        : m_Name(name)
    {
    }
};

class TableName : public Expr {
    std::optional<std::string> m_SchemaName;

    std::string m_TableName;

public:
    // One constructor for schema AND table names
    TableName(std::string schemaName, std::string tableName)
        : m_SchemaName(schemaName)
        , m_TableName(tableName)
    {
    }

    // One constructor for only a table name
    TableName(std::string tableName)
        : m_SchemaName(std::nullopt)
        , m_TableName(tableName)
    {
    }
};

class ColumnName : public Expr {
    std::optional<std::string> m_Schema;

    std::optional<std::string> m_Table;

    std::string m_Name;

public:
    ColumnName(std::string name)
        : m_Name(name)
        , m_Table(std::nullopt)
        , m_Schema(std::nullopt)
    {
    }

    ColumnName(std::string table, std::string name)
        : m_Name(name)
        , m_Table(table)
        , m_Schema(std::nullopt)
    {
    }

    ColumnName(std::string schema, std::string table, std::string name)
        : m_Name(name)
        , m_Table(table)
        , m_Schema(schema)
    {
    }

public:
    ColumnName() { }
};

class BinaryExpression : Expr {
    // Cas Récursifs
    std::optional<std::unique_ptr<BinaryExpression>> m_Lhs;
    std::optional<std::unique_ptr<BinaryExpression>> m_Rhs;

    // Cas de base
    std::optional<std::unique_ptr<Expr>> m_ExprLeft;
    std::optional<std::unique_ptr<Expr>> m_ExprRight;

    // Opérateur
    LogicalOperator m_Op;

public:
    // Cas de base seulement:   a = b
    BinaryExpression(LogicalOperator op, Expr* exprLeft, Expr* exprRight)

        : m_Op(op)
        , m_ExprLeft(exprLeft)
        , m_ExprRight(exprRight)
        , m_Lhs(std::nullopt)
        , m_Rhs(std::nullopt)
    {
    }

    // Un ou deux cas récursifs: a = b OR b = c
    BinaryExpression(LogicalOperator op, BinaryExpression* lhs = nullptr,
        BinaryExpression* rhs = nullptr)
        : m_Op(op)
        , m_ExprLeft(std::nullopt)
        , m_ExprRight(std::nullopt)
        , m_Lhs(lhs)
        , m_Rhs(rhs)
    {
    }
};

} // namespace parsing

#endif // !EXPRESSION_H
