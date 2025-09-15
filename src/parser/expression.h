#include <format>
#include <memory>
#include <optional>
#include <string>

#include "../lexer/tokenizer.h"

#ifndef EXPRESSION_H

#define EXPRESSION_H

namespace Database::Parsing {

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

enum class AggrFuncType {
    AVG_F,
    COUNT_F,
    MAX_F,
    MIN_F,
    SUM_F
};

enum class ColumnType { NULL_C,
    INTEGER_C,
    REAL_C,
    TEXT_C };

class Expr {
    virtual std::string Print() const { return this->Print(); };
};

std::ostream& operator<<(std::ostream& os, const Expr& select_field);

template <typename T>
class LitteralValue : public Expr {
    ColumnType m_Type;

    T m_Data;

public:
    LitteralValue() = default;

    LitteralValue(ColumnType type, T data)
        : m_Type(type)
        , m_Data(data)
    {
    }

    T getData() const { return m_Data; }

    ColumnType getColumnType() const { return m_Type; }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const LitteralValue<T>& litt_val);

class SchemaName : public Expr {
    std::string m_Name;

public:
    SchemaName(std::string name)
        : m_Name(name)
    {
    }

    ~SchemaName() = default;

    std::string Print() const override
    {
        return m_Name;
    }

    std::string getSchemaName() const { return m_Name; }

    static SchemaName* ParseSchemaName(Lexing::Tokenizer* t);
};

std::ostream& operator<<(std::ostream& os, const SchemaName& schema);

class TableName : public Expr {

    std::string m_Name;

    std::optional<std::string> m_SchemaName;

public:
    // One constructor for only a table name
    TableName(std::string name)
        : m_Name(name)
        , m_SchemaName(std::nullopt)
    {
    }

    // One constructor for schema AND table names
    TableName(std::string name, std::string schema_name)
        : m_Name(name)
        , m_SchemaName(schema_name)
    {
    }

    ~TableName() = default;

    static TableName* ParseTableName(Lexing::Tokenizer* t);

    std::string getTableName() const { return m_Name; }

    std::string Print() const override

    {
        std::string res;
        if (m_SchemaName.has_value())
            res = std::format("{}.{}", m_Name, m_SchemaName.value());
        else
            res = m_Name;

        return res;
    }
};

std::ostream& operator<<(std::ostream& os, const TableName& table);

class ColumnName : public Expr {
    std::string m_Name;

    std::optional<std::string> m_TableName;

    std::optional<std::string> m_SchemaName;

public:
    ColumnName() = default;

    ColumnName(std::string name)
        : m_Name(name)
        , m_TableName(std::nullopt)
        , m_SchemaName(std::nullopt)
    {
    }

    ColumnName(std::string name, std::string table)
        : m_Name(name)
        , m_TableName(table)
        , m_SchemaName(std::nullopt)
    {
    }

    ColumnName(std::string name, std::string table, std::string schema)
        : m_Name(name)
        , m_TableName(table)
        , m_SchemaName(schema)
    {
    }

    ~ColumnName() = default;

    std::string getColumnName() const { return m_Name; }

    static ColumnName* ParseColumnName(Lexing::Tokenizer* t);

    std::string Print() const override
    {

        std::string res;
        if (m_SchemaName.has_value())
            res = std::format("{}.{}.{}", m_SchemaName.value(), m_TableName.value(), m_Name);
        else if (m_TableName.has_value())
            res = std::format("{}.{}", m_TableName.value(), m_Name);
        else
            res = m_Name;

        return res;
    }
};

std::ostream& operator<<(std::ostream& os, const ColumnName& column);

AggrFuncType ParseAggregateFunctionType(Lexing::Tokenizer* t);

class AggregateFunction {
    AggrFuncType m_Type;

    bool m_All;

    ColumnName* m_ColumnName;

public:
    AggregateFunction() = default;

    AggregateFunction(AggrFuncType type, ColumnName* col, bool all = false)
        : m_Type(type)
        , m_All(all)
        , m_ColumnName(col)
    {
    }

    bool isAll() { return m_All; }

    AggrFuncType getType() { return m_Type; }

    ColumnName* getColumnName() { return m_ColumnName; }

    static std::variant<AggregateFunction*, Errors::Error> ParseAggregateFunction(Lexing::Tokenizer* t);
};

class BinaryExpression : Expr {
    // Opérateur
    std::optional<LogicalOperator> m_Op;

public:
    using BinaryExpressionMember = std::variant<BinaryExpression*, ColumnName*, LitteralValue<int>*, LitteralValue<std::string>*>;
    // Cas Récursifs
    //
    BinaryExpressionMember m_Lhs;
    BinaryExpressionMember m_Rhs;

    // Un ou deux cas récursifs: a = b OR b = c
    BinaryExpression(BinaryExpressionMember lhs, LogicalOperator op,
        BinaryExpressionMember rhs)
        : m_Op(op)
        , m_Lhs(lhs)
        , m_Rhs(rhs)
    {
    }

    static BinaryExpression* ParseBinaryExpression(Lexing::Tokenizer* t);

    static BinaryExpressionMember ParseMember(Lexing::Tokenizer* t);

    auto Op() -> std::optional<LogicalOperator>& { return m_Op; }

    auto Op() const -> const std::optional<LogicalOperator>& { return m_Op; }
};

std::ostream& operator<<(std::ostream& os, const BinaryExpression& binary_expr);

} // namespace parsing

#endif // !EXPRESSION_H
