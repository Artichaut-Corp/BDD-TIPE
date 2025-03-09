#include <algorithm>
#include <assert.h>
#include <memory>
#include <optional>
#include <vector>

#include "../lexer/tokenizer.h"
#include "expression.h"

#ifndef DML_H

#define DML_H

namespace Compiler::Parsing {

enum class JoinType {
    DEFAULT_J,
    INNER_J,
    LEFT_J,
    LEFT_OUTER_J,
    RIGHT_J,
    RIGHT_OUTER_J,
    FULL_J,
    CROSS_J,
};

std::variant<JoinType, Errors::Error> ParseJoinType(Lexing::Tokenizer* t);

// Sous-types utilisés par les statements ci-dessous

class Assignment {
    ColumnName m_Column;
    Expr m_Expr;
};

// The item after 'order' or 'group'
class ByItem {
    Expr m_Expr;

    bool m_Desc;

public:
    ByItem(Expr expr, bool desc)
        : m_Expr(expr)
        , m_Desc(desc)
    {
    }
};

enum class AggrFuncType {
    AVG_F,
    COUNT_F,
    MAX_F,
    MIN_F,
    SUM_F
};

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

    static std::variant<AggregateFunction*, Errors::Error> ParseAggregateFunction(Lexing::Tokenizer* t);
};

class GroupByClause {
    std::vector<ByItem> m_Items;
};

class HavingClause {
    Expr m_Expr;
};

class OnCondition {
    std::unique_ptr<BinaryExpression> m_Expr;

public:
    OnCondition(BinaryExpression* expr)
        : m_Expr(std::unique_ptr<BinaryExpression>(expr))
    {
    }
};

class Join {
    JoinType m_Type;

    // Left Table
    std::unique_ptr<Expr> m_Left;

    // Right Table
    std::unique_ptr<Expr> m_Right;

    std::unique_ptr<OnCondition> m_On;

    // Could implement using clause

public:
    Join(JoinType type, Expr* left, Expr* right, OnCondition* on)
        : m_Type(type)
        , m_Left(std::unique_ptr<Expr>(left))
        , m_Right(std::unique_ptr<Expr>(right))
        , m_On(std::unique_ptr<OnCondition>(on))
    {
    }
};

class SelectField {
    // If '*' false then Expr is true
    bool m_WildCard = false;

    // If Expr is false then m_WildCard is true
    std::optional<ColumnName> m_Field;

public:
    SelectField() = default;

    SelectField(bool wildcard, ColumnName* field)
        : m_WildCard(wildcard)
    {
        if (field == nullptr)
            m_Field = std::nullopt;
        else
            m_Field = *field;
    }

    SelectField(ColumnName* field)
        : m_WildCard(false)
        , m_Field(*field)
    {
    }

    static std::variant<SelectField*, Errors::Error> ParseSelectField(Lexing::Tokenizer* t);

    std::string Print() const
    {
        std::string res;

        if (m_WildCard)
            res = "WildCard";
        else {
            if (m_Field.has_value())
                res = m_Field.value().Print();
            else
                res = "bizarre";
        }

        return res;
    }
};

std::ostream& operator<<(std::ostream& os, const SelectField& select_field);

class FieldsList {
    std::vector<std::variant<SelectField, AggregateFunction>> m_Fields;

    void AddField(SelectField* s)
    {
        m_Fields.emplace_back(*s);
    }

    void AddField(AggregateFunction* f)
    {
        m_Fields.emplace_back(*f);
    }

public:
    // Cas de *
    FieldsList(bool def)
    {
        m_Fields = std::vector<std::variant<SelectField, AggregateFunction>>();

        m_Fields.emplace_back(SelectField());
    }

    FieldsList()
    {
        m_Fields = std::vector<std::variant<SelectField, AggregateFunction>>();
    }

    static std::variant<FieldsList*, Errors::Error> ParseFieldsList(Lexing::Tokenizer* t);

    std::string Print() const
    {
        std::string res;

        return res;
    }
};

std::ostream& operator<<(std::ostream& os, const FieldsList& fields_list);

class TableRefsClause {
    std::unique_ptr<Join> m_Table;

public:
    TableRefsClause(Join* table)
        : m_Table(std::unique_ptr<Join>(table))
    {
    }
};

class Limit {
    std::unique_ptr<Expr> m_Count;
    std::unique_ptr<Expr> m_Offset;

public:
    Limit(int count)
        : m_Count(new LitteralValue<int>(ColumnType::INTEGER_C, count))
        , m_Offset(new LitteralValue<int>(ColumnType::INTEGER_C, 0))
    {
    }

    Limit(int count, int offset)
        : m_Count(new LitteralValue<int>(ColumnType::INTEGER_C, count))
        , m_Offset(new LitteralValue<int>(ColumnType::INTEGER_C, offset))
    {
    }

    static std::variant<Limit*, Errors::Error> ParseLimit(Lexing::Tokenizer* t);
};

class OrderByClause {
    std::vector<ByItem> m_Items;

    void AddItem(ByItem* b)
    {
        m_Items.push_back(*b);
    }

public:
    OrderByClause()
    {
        m_Items.reserve(1);
    }

    static std::variant<OrderByClause*, Errors::Error> ParseOrderBy(Lexing::Tokenizer* t);
};

class WhereClause {
    std::unique_ptr<BinaryExpression> m_Condition;

public:
    WhereClause(BinaryExpression* cond)
        : m_Condition(std::unique_ptr<BinaryExpression>(cond))
    {
    }

    static WhereClause* ParseWhere(Lexing::Tokenizer* t);
};

/* Statements permettant de modifier les données (DML)
 * Chaque classe contient les informations qui lui sont relatives
 * et une méthode qui prendra le flux de token pour créer un nouveau statement
 */

// DELETE FROM ..
class DeleteStmt {
    // https://www.sqlite.org/lang_delete.html

    // Table à supprimer
    std::unique_ptr<Expr> m_Table;

    // Conditions
    std::optional<std::unique_ptr<WhereClause>> m_Where;

public:
    DeleteStmt(Expr* table, WhereClause* where)
    {
        m_Table = std::unique_ptr<Expr>(table);
        m_Where = std::make_optional<std::unique_ptr<WhereClause>>(where);
    }

    static DeleteStmt* ParseDelete(Lexing::Tokenizer* t);
};

// INSERT INTO
class InsertStmt {
    // Default Values
    bool m_Default;

    // Table to insert into
    std::unique_ptr<TableName> m_Table;

    // Define and optional order ( .., .., )
    std::optional<std::vector<ColumnName>> m_Order;

    // Data, un autre type pourrait probablement
    //  mieux convenir
    std::optional<std::vector<std::vector<Expr>>> m_Data;

    // VALUES

public:
    // Default Case
    InsertStmt(TableName* name, bool def = true)
        : m_Table(name)
        , m_Default(def)

    {
    }

    InsertStmt(TableName* name, bool def, std::vector<std::vector<Expr>> data)
        : m_Table(name)
        , m_Default(def)
        , m_Data(data)
    {
    }

    InsertStmt(TableName* name, bool def, std::vector<ColumnName> order, std::vector<std::vector<Expr>> data)
        : m_Table(name)
        , m_Default(def)
        , m_Data(data)
        , m_Order(order)
    {
    }

    static InsertStmt* ParseInsert(Lexing::Tokenizer* t);
};

// UPDATE
class UpdateStmt {
    // TODO
    //
public:
    static UpdateStmt* ParseUpdate(Lexing::Tokenizer* t);
};

// SELECT ..
class SelectStmt {
    // Disctinct or not
    bool m_Distinct;

    // * or a list of attributes
    std::unique_ptr<FieldsList> m_Fields;

    /*
    // FROM .. (may have joins), pas utilisé tant que l'on ne parse pas les JOIN
    std::unique_ptr<TableRefsClause> m_From;

    remplacé par m_Table ci dessous
    */

    std::unique_ptr<TableName> m_Table;

    // WHERE expr
    std::optional<std::unique_ptr<WhereClause>> m_Where;

    // LIMIT
    std::optional<std::unique_ptr<Limit>> m_Limit;

    // ORDER BY
    std::optional<std::unique_ptr<OrderByClause>> m_OrderBy;

    // Ces deux là s'utilisent avec les fonctions qu'on implémente pas pour l'instant

    // GROUP BY
    std::optional<std::unique_ptr<GroupByClause>> m_GroupBy;

    // HAVING expr
    std::optional<std::unique_ptr<HavingClause>> m_Having;

public:
    SelectStmt(bool distinct, FieldsList* fields_list, TableName* table)
        : m_Distinct(distinct)
        , m_Fields(std::unique_ptr<FieldsList>(fields_list))
        //, m_From(std::make_unique<TableRefsClause>(tables))
        , m_Table(std::unique_ptr<TableName>(table))
        , m_Where(std::nullopt)
        , m_Limit(std::nullopt)
        , m_OrderBy(std::nullopt)
        , m_GroupBy(std::nullopt)
        , m_Having(std::nullopt)
    {
    }

    SelectStmt(bool distinct, FieldsList* fields_list, TableName* table, WhereClause* where, Limit* limit, OrderByClause* order_by)
        : m_Distinct(distinct)
        , m_Fields(std::unique_ptr<FieldsList>(fields_list))
        //, m_From(std::make_unique<TableRefsClause>(tables))
        , m_Table(std::unique_ptr<TableName>(table))
        , m_Where(std::unique_ptr<WhereClause>(where))
        , m_Limit(std::unique_ptr<Limit>(limit))
        , m_OrderBy(std::unique_ptr<OrderByClause>(order_by))
        , m_GroupBy(std::nullopt)
        , m_Having(std::nullopt)
    {
    }

    static SelectStmt* ParseSelect(Lexing::Tokenizer* t);
};
} // namespace parsing

#endif // !DML_H
