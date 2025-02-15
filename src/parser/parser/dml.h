#include <assert.h>
#include <memory>
#include <vector>

#include "../lexer/tokenizer.h"
#include "expression.h"

#ifndef DML_H

#define DML_H

namespace Compiler::Parsing {

enum class JoinType { LEFT,
    RIGHT };

// Sous-types utilisés par les statements ci-dessous

class Assignment {
    ColumnName m_Column;
    Expr m_Expr;
};

// The item after 'order' or 'group'
class ByItem {
    Expr m_Expr;

    bool m_Desc;
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
    bool m_WildCard;

    // If Expr is false then m_WildCard is true
    Expr m_Field;
};

class FieldsList {
    std::vector<SelectField> m_Fields;
};

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
};

class OrderByClause {
    std::vector<ByItem> m_Items;
};

class WhereClause {
    std::unique_ptr<BinaryExpression> m_Condition;

public:
    WhereClause(BinaryExpression* cond)
        : m_Condition(std::unique_ptr<BinaryExpression>(cond))
    {
    }

    static WhereClause* ParseWhere(Lexing::Tokenizer* t)
    {
        return new WhereClause(BinaryExpression::ParseBinaryExpression(t));
    }
};

/* Statements permettant de modifier les données (DML)
 * Chaque classe contient les informations qui lui sont relatives
 * et une méthode qui prendra le flux de token pour créer un nouveau statement
 */
// DELETE FROM ..
class DeleteStmt {
    // TODO: preciser si c'est tout ce que l'on autorise
    // https://www.sqlite.org/lang_delete.html
    std::unique_ptr<TableName> m_Table;

    std::optional<std::unique_ptr<WhereClause>> m_Where;

public:
    DeleteStmt(TableName* table, WhereClause* where)
    {
        m_Table = std::unique_ptr<TableName>(table);
        m_Where = std::make_optional<std::unique_ptr<WhereClause>>(where);
    }

    static DeleteStmt* ParseDelete(Lexing::Tokenizer* t)
    {
        // Juste pour s'assurer. Attention, on passe ici au prochain élément
        assert(t->next().m_Token == Lexing::DELETE_T);

        auto next = t->next();

        if (next.m_Token != Lexing::FROM_T) {
            throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected 'FROM' after 'DELETE'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }

        next = t->next();

        TableName* table;

        if (next.m_Token == Lexing::STRING_LITT_T || next.m_Token == Lexing::VAR_NAME_T) {

            table = new TableName(next.m_Value);

        } else {

            throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected table name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
        }

        next = t->next();

        if (next.m_Token == Lexing::SEMI_COLON_T) {

            return new DeleteStmt(table, nullptr);

        } else if (next.m_Token == Lexing::WHERE_T) {
            return new DeleteStmt(table, WhereClause::ParseWhere(t));

        } else {
            throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected ';' at the end", 0, 0, Errors::ERROR_ENDLINE);
        }
    }
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
};

// UPDATE
class UpdateStmt {
    // TODO
};

// SELECT ..
class SelectStmt {
    // Disctinct or not
    bool m_Distinct;

    // * or a list of attributes
    std::unique_ptr<FieldsList> m_Fields;

    // FROM .. (may have joins)
    std::unique_ptr<TableRefsClause> m_From;

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
    SelectStmt() { }
};
} // namespace parsing

#endif // !DML_H
