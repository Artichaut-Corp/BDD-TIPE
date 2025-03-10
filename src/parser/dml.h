#include <assert.h>
#include <memory>
#include <optional>
#include <vector>

#include "../lexer/tokenizer.h"
#include "expression.h"

#ifndef DML_H

#define DML_H

namespace Database::Parsing {

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

    bool isDesc() const { return m_Desc; }

    Expr getExpr() const { return m_Expr; }
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

public:
    // If Expr is false then m_WildCard is true
    std::optional<ColumnName> m_Field;

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

    bool isWildCard() { return m_WildCard; }

    static std::variant<SelectField*, Errors::Error> ParseSelectField(Lexing::Tokenizer* t);
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

    std::string Print() const
    {
        std::string res;

        return res;
    }

    std::vector<std::variant<SelectField, AggregateFunction>> getField() const { return m_Fields; }

    static std::variant<FieldsList*, Errors::Error> ParseFieldsList(Lexing::Tokenizer* t);
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

    Expr* getCount() const { return m_Count.get(); }

    Expr* getOffset() const { return m_Offset.get(); }

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

    std::vector<ByItem> getByItems() const { return m_Items; }

    static std::variant<OrderByClause*, Errors::Error> ParseOrderBy(Lexing::Tokenizer* t);
};

class WhereClause {
    std::unique_ptr<BinaryExpression> m_Condition;

public:
    WhereClause(BinaryExpression* cond)
        : m_Condition(std::unique_ptr<BinaryExpression>(cond))
    {
    }

    BinaryExpression* getCondition() const { return m_Condition.get(); }

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

    Expr* getTable() const
    {
        return m_Table.get();
    }

    WhereClause* getWhere() const
    {
        if (m_Where.has_value()) {
            return m_Where.value().get();
        }
        return nullptr;
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

    bool isDefault() const
    {
        return m_Default;
    }

    TableName* getTable() const
    {
        return m_Table.get();
    }

    const std::optional<std::vector<ColumnName>>& getOrder() const
    {
        return m_Order;
    }

    const std::optional<std::vector<std::vector<Expr>>>& getData() const
    {
        return m_Data;
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

    bool isDistinct() const
    {
        return m_Distinct;
    }

    FieldsList* getFields() const
    {
        return m_Fields.get();
    }

    TableName* getTable() const
    {
        return m_Table.get();
    }

    WhereClause* getWhere() const
    {
        if (m_Where.has_value()) {
            return m_Where.value().get();
        }
        return nullptr;
    }

    Limit* getLimit() const
    {
        if (m_Limit.has_value()) {
            return m_Limit.value().get();
        }
        return nullptr;
    }

    OrderByClause* getOrderBy() const
    {
        if (m_OrderBy.has_value()) {
            return m_OrderBy.value().get();
        }
        return nullptr;
    }

    GroupByClause* getGroupBy() const
    {
        if (m_GroupBy.has_value()) {
            return m_GroupBy.value().get();
        }
        return nullptr;
    }

    HavingClause* getHaving() const
    {
        if (m_Having.has_value()) {
            return m_Having.value().get();
        }
        return nullptr;
    }

    static SelectStmt* ParseSelect(Lexing::Tokenizer* t);
};
} // namespace parsing

#endif // !DML_H
