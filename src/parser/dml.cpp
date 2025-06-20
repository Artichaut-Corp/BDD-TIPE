#include "dml.h"
#include <cassert>
#include <string>
#include <variant>

namespace Database::Parsing {

bool isJoin(const Lexing::TokenType tok)
{
    return tok == Lexing::JOIN_T
        || tok == Lexing::LEFT_T
        || tok == Lexing::RIGHT_T
        || tok == Lexing::INNER_T
        || tok == Lexing::OUTER_T
        || tok == Lexing::FULL_T
        || tok == Lexing::CROSS_T;
}

std::variant<JoinType, Errors::Error> ParseJoinType(Lexing::Tokenizer* t)
{

    Lexing::Token tok = t->peek();

    switch (tok.m_Token) {
    case Lexing::JOIN_T:
        t->next();

        return JoinType::INNER_J;
    case Lexing::LEFT_T: {
        tok = t->next();

        if (tok.m_Token == Lexing::OUTER_T) {
            t->next();

            return JoinType::LEFT_OUTER_J;
        } else if (tok.m_Token == Lexing::JOIN_T) {
            t->next();
            return JoinType::LEFT_J;
        } else {

            return Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'JOIN' or 'OUTER' after 'LEFT'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }
    };
    case Lexing::RIGHT_T: {
        tok = t->next();

        if (tok.m_Token == Lexing::OUTER_T) {
            t->next();

            return JoinType::RIGHT_OUTER_J;
        } else if (tok.m_Token == Lexing::JOIN_T) {
            t->next();
            return JoinType::RIGHT_J;
        } else {

            return Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'JOIN' or 'OUTER' after 'RIGHT'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }
    };
    case Lexing::INNER_T: {
        tok = t->next();

        if (tok.m_Token != Lexing::JOIN_T) {

            return Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'JOIN' after 'INNER'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }

        t->next();

        return JoinType::INNER_J;
    }

    case Lexing::OUTER_T: {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Unexpected 'OUTER' keyword", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
    }
    case Lexing::FULL_T: {
        tok = t->next();

        if (tok.m_Token == Lexing::OUTER_T) {
            t->next();

            return JoinType::FULL_OUTER_J;
        } else if (tok.m_Token == Lexing::JOIN_T) {
            t->next();

            return JoinType::FULL_J;
        } else {

            return Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'JOIN' or 'OUTER' after 'RIGHT'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }
    };
    case Lexing::CROSS_T: {
        tok = t->next();

        if (tok.m_Token != Lexing::JOIN_T) {

            return Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'JOIN' after 'CROSS'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }

        t->next();

        return JoinType::CROSS_J;
    }
    default:
        return Errors::Error(Errors::ErrorType::RuntimeError, "Unexpected Call to 'ParseJoinType'", 0, 0, Errors::ERROR_UNEXPECTED_CALL_TO_FUNCTION);
    }
}

Join* Join::ParseJoin(Lexing::Tokenizer* t)
{
    auto next = t->peek();

    auto type_v = ParseJoinType(t);

    if (std::holds_alternative<Errors::Error>(type_v)) {
        throw std::get<Errors::Error>(type_v);
    }

    next = t->peek();

    TableName* table = nullptr;

    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected table name after 'JOIN'", 0, 0, Errors::ERROR_EXPECTED_EXPRESSION);
    }

    table = TableName::ParseTableName(t);

    next = t->next();

    if (next.m_Token != Lexing::ON_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'ON' condition after 'JOIN'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }

    next = t->peek();

    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected column name in 'JOIN' condition", 0, 0, Errors::ERROR_EXPECTED_EXPRESSION);
    }

    ColumnName* lhs = ColumnName::ParseColumnName(t);

    next = t->peek();

    if (next.m_Token != Lexing::EQ_OP_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected '=' in 'JOIN' condition", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
    }

    auto op = ParseLogicalOperator(t);

    if (std::holds_alternative<Errors::Error>(op)) {
        throw std::get<Errors::Error>(op);
    }

    if (std::get<LogicalOperator>(op) != LogicalOperator::EQ) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected '=' in 'JOIN' condition", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
    }

    next = t->peek();

    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected column name in 'JOIN' condition", 0, 0, Errors::ERROR_EXPECTED_EXPRESSION);
    }

    ColumnName* rhs = ColumnName::ParseColumnName(t);

    return new Join(std::get<JoinType>(type_v), table, lhs, rhs);
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

    if (next.m_Token != Lexing::INTO_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'INTO' after 'INSERT'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }

    next = t->peek();

    if (next.m_Token != Lexing::VAR_NAME_T) {

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    TableName* table = TableName::ParseTableName(t);

    next = t->next();

    if (next.m_Token == Lexing::DEFAULT_T) {
        next = t->next();

        if (next.m_Token != Lexing::VALUES_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'VALUES' after 'DEFAULT' in 'INSERT' statement", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }

        next = t->peek();

        if (next.m_Token == Lexing::SEMI_COLON_T) {
            return new InsertStmt(table);
        }

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected ';' at the end", 0, 0, Errors::ERROR_ENDLINE);
    } else if (next.m_Token == Lexing::VALUES_T) {

        next = t->next();

        if (next.m_Token != Lexing::LPAREN_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected a '('", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
        }

        std::vector<Expr> data;

        do {
            next = t->next();

            if (next.m_Token != Lexing::STRING_LITT_T && next.m_Token != Lexing::NUM_LITT_T) {

                throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected litteral values in 'INSERT' statement", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
            } else {

                if (next.m_Token == Lexing::STRING_LITT_T) {

                    LitteralValue<std::string> val = LitteralValue<std::string>(ColumnType::TEXT_C, next.m_Value);

                    data.push_back(val);
                }

                else {
                    LitteralValue<int> val = LitteralValue<int>(ColumnType::INTEGER_C, std::stoi(next.m_Value));

                    data.push_back(val);
                }
            }

            next = t->peek();

            if (next.m_Token != Lexing::COMMA_T) {
                break;
            } else {
                // Consommer la virgule
                t->next();
            }

        } while (1);

        next = t->next();

        if (next.m_Token != Lexing::RPAREN_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected a ')'", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
        }

        return new InsertStmt(table, false, data);
    } else {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'VALUES (...)' or 'DEFAULT' in 'INSERT' statement", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }
}

// Juste remplacer par DELETE puis INSERT ça marche aussi bien
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

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table name after 'FROM' in a 'SELECT' Statment", 0, 0, Errors::ERROR_EXPECTED_EXPRESSION);
    }

    next = t->peek();

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct, std::get<FieldsList*>(field), table);
    }

    // JOIN

    std::vector<Join>* joins = new std::vector<Join>();

    joins->reserve(1);

    while (isJoin(next.m_Token)) {

        joins->emplace_back(*Join::ParseJoin(t));

        next = t->peek();
    }

    // WHERE

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct, std::get<FieldsList*>(field), table, joins);
    }

    std::variant<WhereClause*, Errors::Error> where = nullptr;

    if (next.m_Token == Lexing::WHERE_T) {
        t->next();

        where = WhereClause::ParseWhere(t);

        next = t->peek();
    }

    // ORDER BY

    if (next.m_Token == Lexing::SEMI_COLON_T) {

        return new SelectStmt(distinct,
            std::get<FieldsList*>(field),
            table,
            joins,
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
            joins,
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
            joins,
            std::get<WhereClause*>(where),
            std::get<Limit*>(limit),
            std::get<OrderByClause*>(order_by));
    } else {

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected ';' at the end", 0, 0, Errors::ERROR_ENDLINE);
    }
}

Transaction* Transaction::ParseTransaction(Lexing::Tokenizer* t)
{
    assert(t->next().m_Token == Lexing::TRANSACTION_T);

    auto next = t->peek();

    std::cout << next << "\n";

    TableName* name;

    std::vector<std::vector<Expr>> data;

    while (next.m_Token != Lexing::END_T) {

        std::cout << next << "\n";

        assert(t->next().m_Token == Lexing::INSERT_T);

        auto next = t->next();

        if (next.m_Token != Lexing::INTO_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'INTO' after 'INSERT'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
        }

        next = t->peek();

        if (next.m_Token != Lexing::VAR_NAME_T) {

            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
        }

        // On pourrait optimiser le parsing en n'analysant pas les
        // noms de table que l'on connait déjà

        name = TableName::ParseTableName(t);

        next = t->next();

        if (next.m_Token != Lexing::VALUES_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'VALUES' in transactionnal 'INSERT' statement ", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
        }

        next = t->next();

        if (next.m_Token != Lexing::LPAREN_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected a '('", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
        }

        std::vector<Expr> values;

        do {
            next = t->next();

            if (next.m_Token != Lexing::STRING_LITT_T && next.m_Token != Lexing::NUM_LITT_T) {

                throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected litteral values in 'INSERT' statement", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
            }

            if (next.m_Token == Lexing::STRING_LITT_T) {

                LitteralValue<std::string> val = LitteralValue<std::string>(ColumnType::TEXT_C, next.m_Value);

                values.push_back(val);
            }

            else {
                LitteralValue<int> val = LitteralValue<int>(ColumnType::INTEGER_C, std::stoi(next.m_Value));

                values.push_back(val);
            }

            next = t->peek();

            if (next.m_Token != Lexing::COMMA_T) {
                break;
            } else {
                // Consommer la virgule
                t->next();
            }
        } while (1);

        data.push_back(values);

        next = t->next();

        if (next.m_Token != Lexing::RPAREN_T) {
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected a ')'", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
        }
    }

    if (next.m_Token != Lexing::END_T) {

        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected 'END' after 'TRANSACTION'", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
    }

    next = t->next();

    if (next.m_Token != Lexing::SEMI_COLON_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected ';' at the end", 0, 0, Errors::ERROR_ENDLINE);
    }

    return new Transaction(name, data);
}

} // namespace parsing
