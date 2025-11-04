#include <format>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>


#include "../algebrizer_types.h"
#include "../data_process_system/namingsystem.h"
#include "../lexer/tokenizer.h"
#include "../utils.h"

#ifndef EXPRESSION_H

#define EXPRESSION_H

namespace Database::Parsing {

std::variant<std::string, Errors::Error> ParseAs(Lexing::Tokenizer* t);

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

std::ostream& operator<<(std::ostream& out, const LogicalOperator& member);

enum class AggrFuncType {
    AVG_F,
    COUNT_F,
    MAX_F,
    MIN_F,
    SUM_F,

    NOTHING_F // utile que pour le type Final pour simplifier le code
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

using Aliases = std::unordered_set<std::string>;

class TableName : public Expr {

    std::string m_Name;

    std::optional<std::string> m_SchemaName;

    Aliases m_Aliases;

public:
    // One constructor for only a table name
    TableName(std::string name, Aliases aka)
        : m_Name(name)
        , m_SchemaName(std::nullopt)
        , m_Aliases(aka)
    {
    }

    // One constructor for schema AND table names
    TableName(std::string name, std::string schema_name, Aliases aka)
        : m_Name(name)
        , m_SchemaName(schema_name)
        , m_Aliases(aka)
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

    Aliases* GetAlias() { return &m_Aliases; };
};

std::ostream& operator<<(std::ostream& os, const TableName& table);

class ColumnName : public Expr {
    std::string m_Name;

    std::optional<std::string> m_TableName;

    std::optional<std::string> m_SchemaName;

    Aliases m_Aliases;

public:
    ColumnName() = default;

    ColumnName(std::string name, Aliases aka)
        : m_Name(name)
        , m_TableName(std::nullopt)
        , m_SchemaName(std::nullopt)
        , m_Aliases(aka)
    {
    }

    ColumnName(std::string name, std::string table, Aliases aka)
        : m_Name(name)
        , m_TableName(table)
        , m_SchemaName(std::nullopt)
        , m_Aliases(aka)
    {
    }

    ColumnName(std::string name, std::string table, std::string schema, Aliases aka)
        : m_Name(name)
        , m_TableName(table)
        , m_SchemaName(schema)
        , m_Aliases(aka)
    {
    }

    ~ColumnName() = default;

    std::string getColumnName() const
    {

        return m_Name;
    }

    void SetTable(std::string NouvelleTable) { m_TableName = NouvelleTable; }

    bool HaveSchema() { return m_SchemaName.has_value(); };

    bool HaveTable() const { return m_TableName.has_value(); };

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
    Aliases* GetAlias() { return &m_Aliases; }

    std::string GetTable() const { return m_TableName.value(); }
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

using ClauseMember = std::variant<QueryPlanning::ColonneNamesSet*, ColumnData>;

std::ostream& operator<<(std::ostream& out, const ClauseMember& member);

class Clause {

    LogicalOperator m_Op;

    ClauseMember m_Lhs;
    ClauseMember m_Rhs;
    std::unordered_set<QueryPlanning::ColonneNamesSet*>* m_ColumnUsed;
    std::pair<float,float> m_InfoSelectivité;


public:
    Clause(LogicalOperator op, ClauseMember lhs, ClauseMember rhs, std::unordered_set<QueryPlanning::ColonneNamesSet*>* col_used)
        : m_Op(op)
        , m_Lhs(lhs)
        , m_Rhs(rhs)
        , m_ColumnUsed(col_used)
    {
    }

    auto Lhs() const -> ClauseMember { return m_Lhs; }
    auto Rhs() const -> ClauseMember { return m_Rhs; }
    auto Op() const -> LogicalOperator { return m_Op; }
    auto Column() -> std::unordered_set<QueryPlanning::ColonneNamesSet*>* { return m_ColumnUsed; }
    auto EditLhs(ClauseMember NewClause) { m_Lhs = NewClause; }
    auto EditRhs(ClauseMember NewClause) { m_Rhs = NewClause; }
    void Print(std::ostream& out);

    static std::pair<ClauseMember, QueryPlanning::ColonneNamesSet*> ParseClauseMember(Lexing::Tokenizer* t);

    static Clause* ParseClause(Lexing::Tokenizer* t);

    void FormatColumnName(QueryPlanning::TableNamesSet* NomTablePrincipale);

    bool Eval(std::unordered_map<std::string, ColumnData*>* CombinaisonATester);

    bool EstimeSelectivite(std::unordered_map<std::string, ColumnData*>* CombinaisonATester);

    float GetSelectivite(){return m_InfoSelectivité.second/m_InfoSelectivité.first;}
};

std::ostream& operator<<(std::ostream& out, const Clause& member);

class BinaryExpression {

public:
    using Condition = std::variant<BinaryExpression*, Clause*, std::monostate>; // monostate représente le noeud vide

    BinaryExpression(LogicalOperator op, Condition lhs, Condition rhs, std::unordered_set<QueryPlanning::ColonneNamesSet*>* col_used)
        : m_Op(op)
        , m_Lhs(lhs)
        , m_Rhs(rhs)
        , m_ColumnUsedBelow(col_used) { };

    // Parsing utilities
    static std::unordered_set<QueryPlanning::ColonneNamesSet*>* MergeColumns(Condition lhs, Condition rhs);

    static BinaryExpression::Condition ParseCondition(Lexing::Tokenizer* t);

    // Data accesing methods

    void PrintCondition(std::ostream& out);
    void PrintConditionalt(std::ostream& out);

    auto Lhs() -> Condition
    {
        return m_Lhs;
    }
    auto Rhs() -> Condition { return m_Rhs; }
    auto Op() -> LogicalOperator { return m_Op; }

    bool IsEmpty(const Condition& cond)
    {
        return std::holds_alternative<std::monostate>(cond);
    }
    auto NullifyLhs() -> void
    {
        m_Lhs = std::monostate {};
        auto right = Rhs();
        if (std::holds_alternative<std::monostate>(right)) {
            m_ColumnUsedBelow = {};
        } else if (std::holds_alternative<Clause*>(right)) {
            m_ColumnUsedBelow = std::get<Clause*>(right)->Column();
        } else {
            m_ColumnUsedBelow = std::get<BinaryExpression*>(right)->Column();
        }
    }
    auto NullifyRhs() -> void
    {
        m_Rhs = std::monostate {};
        auto left = Lhs();
        if (std::holds_alternative<std::monostate>(left)) {
            m_ColumnUsedBelow = {};
        } else if (std::holds_alternative<Clause*>(left)) {
            m_ColumnUsedBelow = std::get<Clause*>(left)->Column();
        } else {
            m_ColumnUsedBelow = std::get<BinaryExpression*>(left)->Column();
        }
    }

    auto Column() -> std::unordered_set<QueryPlanning::ColonneNamesSet*>* { return m_ColumnUsedBelow; }

    // Evaluation methods

    Condition ExtraireCond(std::unordered_set<QueryPlanning::ColonneNamesSet*>* ColonnesAExtraire);

    bool Eval(std::unordered_map<std::string, ColumnData*>* CombinaisonATester);

    bool EstimeSelectivite(std::unordered_map<std::string, ColumnData*>* CombinaisonATester);

    void FormatColumnName(QueryPlanning::TableNamesSet* NomTablePrincipale);

    float OptimiseBinaryExpression();

private:
    // Opérateur, ne peut être que AND / OR
    LogicalOperator m_Op;

    Condition m_Lhs;
    Condition m_Rhs;
    std::unordered_set<QueryPlanning::ColonneNamesSet*>* m_ColumnUsedBelow;

    std::pair<float,float> m_InfoSelectivité;
};

} // namespace parsing

#endif // !EXPRESSION_H
