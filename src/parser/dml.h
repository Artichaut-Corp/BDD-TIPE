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
    INNER_J,
    LEFT_J,
    LEFT_OUTER_J,
    RIGHT_J,
    RIGHT_OUTER_J,
    FULL_J,
    FULL_OUTER_J,
    CROSS_J,
};

bool isJoin(const Lexing::Token tok);

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

    std::unique_ptr<TableName> m_Table;

    // Left Table
    std::unique_ptr<ColumnName> m_Left;

    // Right Table
    std::unique_ptr<ColumnName> m_Right;

public:
    Join(JoinType type, TableName* table, ColumnName* left, ColumnName* right)
        : m_Type(type)
        , m_Table(std::make_unique<TableName>(*table))
        , m_Left(std::make_unique<ColumnName>(*left))
        , m_Right(std::make_unique<ColumnName>(*right))
    {
    }

    Join(const Join& other)
        : m_Type(other.m_Type)
    {
        m_Table = std::unique_ptr<TableName>(other.m_Table.get());

        m_Left = std::unique_ptr<ColumnName>(other.m_Left.get());
        m_Right = std::unique_ptr<ColumnName>(other.m_Right.get());
    }

    ~Join() = default;

    ColumnName* getLeftColumn() const
    {
        return m_Left.get();
    }

    ColumnName* getRightColumn() const
    {
        return m_Right.get();
    }

    TableName* getTable() const
    {
        return m_Table.get();
    }

    JoinType getJoinType() const
    {
        return m_Type;
    }

    static Join* ParseJoin(Lexing::Tokenizer* t);
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

    ColumnName getColumnName() { return m_Field.value(); }

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
    int m_Count;
    int m_Offset;

public:
    Limit(int count)
        : m_Count(count)
        , m_Offset(0)
    {
    }

    Limit(int count, int offset)
        : m_Count(count)
        , m_Offset(offset)
    {
    }

    int getCount() const { return m_Count; }

    int getOffset() const { return m_Offset; }

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
    std::unique_ptr<BinaryExpression::Condition> m_Condition;

public:
    WhereClause(BinaryExpression::Condition* cond)
        : m_Condition(std::unique_ptr<BinaryExpression::Condition>(cond))
    {
    }

    BinaryExpression::Condition* getCondition() const { return m_Condition.get(); }

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
    std::optional<std::unique_ptr<std::vector<ColumnName>>> m_Order;

    // Data, un autre type pourrait probablement
    //  mieux convenir
    std::optional<std::unique_ptr<std::vector<LitteralValue<std::string>>>> m_Data;

    // VALUES

public:
    // Default Case
    InsertStmt(TableName* name, bool def = true)
        : m_Table(name)
        , m_Default(def)
        , m_Data(std::nullopt)

    {
    }

    InsertStmt(TableName* name, bool def, std::vector<LitteralValue<std::string>>* data)
        : m_Table(name)
        , m_Default(def)
        , m_Data(data)
    {
    }

    InsertStmt(TableName* name, bool def, std::vector<LitteralValue<std::string>>* data, std::vector<ColumnName>* order)
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

    const std::optional<std::unique_ptr<std::vector<ColumnName>>>& getOrder() const
    {
        return m_Order;
    }

    const std::optional<std::unique_ptr<std::vector<LitteralValue<std::string>>>>& getData() const
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
    std::unique_ptr<FieldsList> m_Fields; // ce qu'il y'as entre le Select et le From (les colonne et les tables sont mélangé)

    std::unique_ptr<TableName> m_Table; // ce qu'il y'as après le From

    std::optional<std::unique_ptr<std::vector<Join>>> m_Joins; // les join (des jointure dcp)

    // WHERE expr
    std::optional<std::unique_ptr<WhereClause>> m_Where; // les conditions des objets (des projections)

    // LIMIT
    std::optional<std::unique_ptr<Limit>> m_Limit; // les limites (des projections)

    // ORDER BY
    std::optional<std::unique_ptr<OrderByClause>> m_OrderBy; // des tris (et donc des projections)

    // Ces deux là s'utilisent avec les fonctions qu'on implémente pas pour l'instant

    // GROUP BY
    std::optional<std::unique_ptr<GroupByClause>> m_GroupBy;

    // HAVING expr
    std::optional<std::unique_ptr<HavingClause>> m_Having; // projections

public:
    SelectStmt(bool distinct, FieldsList* fields_list, TableName* table)
        : m_Distinct(distinct)
        , m_Fields(std::unique_ptr<FieldsList>(fields_list))
        , m_Table(std::unique_ptr<TableName>(table))
        , m_Joins(std::nullopt)
        , m_Where(std::nullopt)
        , m_Limit(std::nullopt)
        , m_OrderBy(std::nullopt)
        , m_GroupBy(std::nullopt)
        , m_Having(std::nullopt)
    {
    }

    SelectStmt(bool distinct, FieldsList* fields_list, TableName* table, std::vector<Join>* join)
        : m_Distinct(distinct)
        , m_Fields(std::unique_ptr<FieldsList>(fields_list))
        , m_Table(std::unique_ptr<TableName>(table))
        , m_Joins(join)
        , m_Where(std::nullopt)
        , m_Limit(std::nullopt)
        , m_OrderBy(std::nullopt)
        , m_GroupBy(std::nullopt)
        , m_Having(std::nullopt)
    {
    }

    SelectStmt(bool distinct, FieldsList* fields_list, TableName* table, std::vector<Join>* join, WhereClause* where, Limit* limit, OrderByClause* order_by)
        : m_Distinct(distinct)
        , m_Fields(std::unique_ptr<FieldsList>(fields_list))
        , m_Joins(join)
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

    std::vector<Join>* getJoins() const
    {
        if (m_Joins.has_value()) {
            return m_Joins.value().get();
        }

        return nullptr;
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

/* Type of request used to insert data in large batches.
 * Syntax:
 *  TRANSACTION table_name
 *  (col_1, col_2, ..., col_n)
 *  VALUES
 *  (val_11, val_21, ..., val_n1 ),
 *  ....
 *  (val_1n, val_2n, ..., val_nn)
 *  END;
 */
class Transaction {
    std::unique_ptr<TableName> m_Table;

    std::unique_ptr<std::vector<LitteralValue<std::string>>> m_Data;

    std::unique_ptr<std::vector<ColumnName>> m_Order;

    Transaction(TableName* name, std::vector<LitteralValue<std::string>>* data, std::vector<ColumnName>* order)
        : m_Table(name)
        , m_Data(data)
        , m_Order(order)
    {
    }

public:
    static Transaction* ParseTransaction(Lexing::Tokenizer* t);

    TableName* getTable() const
    {
        return m_Table.get();
    }

    const std::unique_ptr<std::vector<LitteralValue<std::string>>>& getData() const
    {
        return m_Data;
    }

    const std::unique_ptr<std::vector<ColumnName>>& getOrder() const
    {
        return m_Order;
    }
};

} // namespace parsing

#endif // !DML_H
