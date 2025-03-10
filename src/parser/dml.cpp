#include "dml.h"
#include <cassert>
#include <stdexcept>
#include <string>
#include <variant>

namespace Compiler::Parsing {

std::variant<JoinType, Errors::Error> ParseJoinType(Lexing::Tokenizer* t)
{
    assert(t->next().m_Token == Lexing::JOIN_T);

    Lexing::Token tok = t->peek();

    switch (tok.m_Token) {
    case Lexing::LEFT_T:
    case Lexing::RIGHT_T:
    case Lexing::INNER_T:
    case Lexing::OUTER_T:
    case Lexing::FULL_T: {
        t->next();
        return JoinType::FULL_J;
    }
    case Lexing::CROSS_T: {
        t->next();
        return JoinType::CROSS_J;
    }
    default:
        return JoinType::DEFAULT_J;
    }
}

AggrFuncType ParseAggregateFunctionType(Lexing::Tokenizer* t)
{
    auto token = t->peek();

    AggrFuncType ret;

    if (token.m_Value == "AVG") {
        ret = AggrFuncType::AVG_F;
    } else if (token.m_Value == "COUNT") {
        ret = AggrFuncType::COUNT_F;
    } else if (token.m_Value == "MAX") {
        ret = AggrFuncType::MAX_F;
    } else if (token.m_Value == "MIN") {
        ret = AggrFuncType::MIN_F;
    } else if (token.m_Value == "SUM") {
        ret = AggrFuncType::SUM_F;
    } else {
        throw std::runtime_error("");
    }

    t->next();

    return ret;
}

std::variant<AggregateFunction*, Errors::Error> AggregateFunction::ParseAggregateFunction(Lexing::Tokenizer* t)
{
    auto token = t->peek();

    if (token.m_Token != Lexing::AGGR_FUNC_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected Aggregate Function Names", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    AggrFuncType type = ParseAggregateFunctionType(t);

    token = t->next();

    if (token.m_Token != Lexing::LPAREN_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected a '('", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
    }

    token = t->peek();

    ColumnName* col;

    bool all = false;

    if (token.m_Token == Lexing::VAR_NAME_T) {
        col = ColumnName::ParseColumnName(t);

    } else if (token.m_Token == Lexing::MATH_OP_T && token.m_Value == "*") {

        all = true;

        t->next();
    } else {

        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected Column Name or '*' in Aggregate Function ", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    token = t->next();

    if (token.m_Token != Lexing::RPAREN_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected a ')'", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
    }

    return new AggregateFunction(type, col, all);
}

// Si cette fonction est appelée, le cas de '*' est déjà traité.
// C'est un peu étrange mais c'est comme ça.
// Ici, on crée un Champ contenant un nom de colonne
std::variant<SelectField*, Errors::Error> SelectField::ParseSelectField(Lexing::Tokenizer* t)
{
    ColumnName* col = ColumnName::ParseColumnName(t);

    return new SelectField(col);
}

std::ostream& operator<<(std::ostream& os, const SelectField& select_field)
{
    os << select_field.Print();

    return os;
}

std::variant<FieldsList*, Errors::Error> FieldsList::ParseFieldsList(Lexing::Tokenizer* t)
{

    Lexing::Token next = t->peek();

    if (next.m_Token == Lexing::MATH_OP_T && next.m_Value == "*") {

        t->next();

        // Retourne juste '*' par défaut
        return new FieldsList(true);
    }

    FieldsList* field_list = new FieldsList();

    do {
        next = t->peek();

        if (next.m_Token == Lexing::VAR_NAME_T) {
            std::variant<SelectField*, Errors::Error> field = SelectField::ParseSelectField(t);

            if (std::holds_alternative<Errors::Error>(field)) {
                return std::get<Errors::Error>(field);
            }

            field_list->AddField(std::get<SelectField*>(field));

            next = t->peek();

            if (next.m_Token != Lexing::COMMA_T) {
                break;
            } else {
                // Consommer la virgule
                t->next();
            }
        } else if (next.m_Token == Lexing::AGGR_FUNC_T) {
            std::variant<AggregateFunction*, Errors::Error> func = AggregateFunction::ParseAggregateFunction(t);

            if (std::holds_alternative<Errors::Error>(func)) {
                return std::get<Errors::Error>(func);
            }

            field_list->AddField(std::get<AggregateFunction*>(func));

            next = t->peek();

            if (next.m_Token != Lexing::COMMA_T) {
                break;
            } else {
                // Consommer la virgule
                t->next();
            }
        } else {
            return Errors::Error(Errors::ErrorType::SyntaxError, "Expected Column Names or Aggregate Functions after 'SELECT' statement", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
        }

    } while (1);

    return field_list;
}

std::ostream& operator<<(std::ostream& os, const FieldsList& fields_list)
{
    os << fields_list.Print();

    return os;
}

std::variant<Limit*, Errors::Error> Limit::ParseLimit(Lexing::Tokenizer* t)
{
    assert(t->next().m_Token == Lexing::LIMIT_T);

    auto tok = t->next();

    if (tok.m_Token != Lexing::NUM_LITT_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected Numerical Value After 'LIMIT'", 0, 0, Errors::ERROR_EXPECTED_EXPRESSION);
    }

    auto next = t->peek();

    if (next.m_Token != Lexing::OFFSET_T) {
        return new Limit(stoi(tok.m_Value));
    }

    // Consomme 'OFFSET'
    t->next();

    next = t->next();

    if (next.m_Token != Lexing::NUM_LITT_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected Numerical Value After 'OFFSET'", 0, 0, Errors::ERROR_EXPECTED_EXPRESSION);
    }

    return new Limit(stoi(tok.m_Value), stoi(next.m_Value));
}

std::variant<OrderByClause*, Errors::Error> OrderByClause::ParseOrderBy(Lexing::Tokenizer* t)
{
    assert(t->next().m_Token == Lexing::ORDER_T);

    auto next = t->next();

    if (next.m_Token != Lexing::BY_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'BY' after 'ORDER'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }

    auto order = new OrderByClause();

    // Créer des items 'BY' tant qu'on trouve des virgules séparatrices
    do {

        // Column Name
        Expr* name = ColumnName::ParseColumnName(t);

        next = t->peek();

        // ASC par défaut
        bool desc = false;

        // Si un keyword apparait on le consomme
        if (next.m_Token == Lexing::DESC_T) {
            t->next();
            desc = true;
        }
        if (next.m_Token == Lexing::ASC_T) {
            t->next();
        }

        order->AddItem(new ByItem(*name, desc));

        next = t->peek();

        if (next.m_Token != Lexing::COMMA_T) {
            break;
        }

        // Consommer la virgule
        t->next();
    } while (1);

    return order;
}

WhereClause* WhereClause::ParseWhere(Lexing::Tokenizer* t)
{
    return new WhereClause(BinaryExpression::ParseBinaryExpression(t));
}

DeleteStmt* DeleteStmt::ParseDelete(Lexing::Tokenizer* t)
{
    // Juste pour s'assurer. Attention, on passe ici au prochain élément
    assert(t->next().m_Token == Lexing::DELETE_T);

    auto next = t->next();

    if (next.m_Token != Lexing::FROM_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'FROM' after 'DELETE'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }

    next = t->peek();

    TableName* table;

    if (next.m_Token == Lexing::VAR_NAME_T) {

        table = TableName::ParseTableName(t);

    } else {

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    next = t->next();

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new DeleteStmt(table, nullptr);

    } else if (next.m_Token == Lexing::WHERE_T) {
        return new DeleteStmt(table, WhereClause::ParseWhere(t));

    } else {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected ';' at the end", 0, 0, Errors::ERROR_ENDLINE);
    }
}

InsertStmt* InsertStmt::ParseInsert(Lexing::Tokenizer* t)
{
    assert(t->next().m_Token == Lexing::INSERT_T);

    auto next = t->next();

    // TODO

    throw Errors::Error(Errors::ErrorType::RuntimeError, "Unimplemented", 0, 0, Errors::ERROR_UNIMPLEMENTED);

    return new InsertStmt(new TableName("Placeholder"));
}

UpdateStmt* UpdateStmt::ParseUpdate(Lexing::Tokenizer* t)
{
    assert(t->next().m_Token == Lexing::UPDATE_T);

    auto next = t->next();

    // TODO

    throw Errors::Error(Errors::ErrorType::RuntimeError, "Unimplemented", 0, 0, Errors::ERROR_UNIMPLEMENTED);

    return new UpdateStmt();
}

SelectStmt* SelectStmt::ParseSelect(Lexing::Tokenizer* t)
{

    assert(t->next().m_Token == Lexing::SELECT_T);

    auto next = t->peek();

    // DISTINCT
    bool distinct = false;

    if (next.m_Token == Lexing::DISTINCT_T) {
        distinct = true;
        t->next();
    }

    // Champs de retour, '*' ou fonctions d'aggregation
    std::variant<FieldsList*, Errors::Error> field = FieldsList::ParseFieldsList(t);

    if (std::holds_alternative<Errors::Error>(field)) {
        throw std::get<Errors::Error>(field);
    }

    next = t->next();

    // FROM
    if (next.m_Token != Lexing::FROM_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'FROM' after 'SELECT'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }

    next = t->peek();

    // Table dans laquelle  on selectionne

    TableName* table = nullptr;

    if (next.m_Token == Lexing::VAR_NAME_T) {

        table = TableName::ParseTableName(t);

    } else {

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table name after 'FROM' in a 'SELECT' Statment", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    next = t->peek();

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct, std::get<FieldsList*>(field), table);
    }

    // JOIN

    // TODO

    // WHERE

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct, std::get<FieldsList*>(field), table);
    }

    std::variant<WhereClause*, Errors::Error> where = nullptr;

    if (next.m_Token == Lexing::WHERE_T) {
        where = WhereClause::ParseWhere(t);

        next = t->peek();
    }

    // ORDER BY

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct,
            std::get<FieldsList*>(field),
            table,
            std::get<WhereClause*>(where),
            nullptr, nullptr);
    }

    std::variant<OrderByClause*, Errors::Error> order_by = nullptr;

    if (next.m_Token == Lexing::ORDER_T) {

        order_by = OrderByClause::ParseOrderBy(t);

        if (std::holds_alternative<Errors::Error>(order_by)) {
            throw std::get<Errors::Error>(order_by);
        }

        next = t->peek();
    }

    // LIMIT

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct,
            std::get<FieldsList*>(field),
            table,
            std::get<WhereClause*>(where),
            nullptr,
            std::get<OrderByClause*>(order_by));
    }

    std::variant<Limit*, Errors::Error> limit = nullptr;

    if (next.m_Token == Lexing::LIMIT_T) {
        limit = Limit::ParseLimit(t);

        if (std::holds_alternative<Errors::Error>(limit)) {
            throw std::get<Errors::Error>(limit);
        }

        next = t->peek();
    }

    // ';'
    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct,
            std::get<FieldsList*>(field),
            table,
            std::get<WhereClause*>(where),
            std::get<Limit*>(limit),
            std::get<OrderByClause*>(order_by));
    } else {

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected ';' at the end", 0, 0, Errors::ERROR_ENDLINE);
    }
}
} // namespace parsing
