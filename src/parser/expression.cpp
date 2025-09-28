#include "expression.h"
#include "../utils.h"
#include "../algebrizer/algebrizer.h"


#include <bits/types/stack_t.h>
#include <cstddef>
#include <memory>
#include <ostream>
#include <type_traits>
#include <unordered_set>
#include <variant>

namespace Database::Parsing {

std::variant<LogicalOperator, Errors::Error> ParseLogicalOperator(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    switch (tok.m_Token) {

    // Cas des charactères
    case Lexing::EQ_OP_T: {

        // On ne consomme pas le token suivant dans le cas où
        // il ne fait pas partie de l'opérateur
        Lexing::Token next = t->peek();

        if (tok.m_Value == ">") {
            if (next.m_Value == "=") {
                t->next();
                return LogicalOperator::GTE;
            }
            return LogicalOperator::GT;
        } else if (tok.m_Value == "<") {

            if (next.m_Value.front() == '=') {
                t->next();
                return LogicalOperator::LTE;
            }
            return LogicalOperator::LT;
        }
        // '=' ou '==' sont équivalents
        else if (tok.m_Value == "=") {
            if (next.m_Value.front() == '=') {
                t->next();
                return LogicalOperator::EQ;
            }
            return LogicalOperator::EQ;
        } else if (tok.m_Value == "!") {
            if (next.m_Value.front() == '=') {
                t->next();
                return LogicalOperator::NE;
            }
            return LogicalOperator::NOT;
        }
        // '&' seul est un opérateur arithmétique
        else if (tok.m_Value == "&") {
            if (next.m_Value.front() == '&') {
                t->next();
                return LogicalOperator::AND;
            } else {
                return Errors::Error(Errors::ErrorType::SyntaxError, "Unexpected '&' in binary expression", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
            }
        }
        // '|' seul est un opérateur arithmétique
        else if (tok.m_Value == "|") {
            if (next.m_Value.front() == '&') {
                t->next();
                return LogicalOperator::AND;
            } else {
                return Errors::Error(Errors::ErrorType::SyntaxError, "Unexpected '|' in binary expression", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
            }
        }
        // Je sait pas trop ce qu'il fait là
        else {
            return Errors::Error(Errors::ErrorType::SyntaxError, "Unexpected symbol in binary expression", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
        }
    }
    // Cas des opérateurs en toute lettre
    case Lexing::AND_T:
        return LogicalOperator::AND;
    case Lexing::OR_T:
        return LogicalOperator::OR;
    case Lexing::NOT_T:
        return LogicalOperator::NOT;
    default:
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected a Logical Operator in Expression", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
    }
}

std::ostream& operator<<(std::ostream& out, const LogicalOperator& op)
{
    switch (op) {
    case LogicalOperator::EQ:
        out << "=";
        break;
    case LogicalOperator::GT:
        out << ">";
        break;
    case LogicalOperator::LT:
        out << "<";
        break;
    case LogicalOperator::GTE:
        out << ">=";
        break;
    case LogicalOperator::LTE:
        out << "<=";
        break;
    case LogicalOperator::NE:
        out << "!=";
        break;
    case LogicalOperator::AND:
        out << "&&";
        break;
    case LogicalOperator::OR:
        out << "||";
        break;
    case LogicalOperator::NOT:
        out << "!";
        break;
    }

    return out;
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

SchemaName* SchemaName::ParseSchemaName(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Schema Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Comme on parse uniquement un nom de schema, on retourne directement
    return new SchemaName(tok.m_Value);
}

std::ostream& operator<<(std::ostream& os, const SchemaName& schema)
{
    os << schema.Print();

    return os;
}

// Retourne soit:
//  - Schema.Table
//  - Table
TableName* TableName::ParseTableName(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Regarde le prochain sans le consommer car il est possible que
    // ce soit un nom de table seul
    Lexing::Token next = t->peek();

    if (next.m_Token != Lexing::DOT_T) {
        return new TableName(tok.m_Value);
    }

    // On peut alors consommer
    t->next();

    // Et regarder le suivant. On peut le consommer directement
    // car il est certain que l'on doive trouver un nom
    next = t->next();

    // Vérification du token suivant
    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Schema + Table
    return new TableName(tok.m_Value, next.m_Value);
}

std::ostream& operator<<(std::ostream& os, const TableName& table)
{
    os << table.Print();

    return os;
}

// Retourne soit:
//  - Schema.Table.Colonne
//  - Table.Colonne
//  - Colonne
ColumnName* ColumnName::ParseColumnName(Lexing::Tokenizer* t)
{

    Lexing::Token tok = t->next();

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Column Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Regarde le prochain sans le consommer car il est possible que
    // ce soit un nom de colonne seul
    Lexing::Token next = t->peek();

    if (next.m_Token != Lexing::DOT_T) {
        // Schema seul
        return new ColumnName(tok.m_Value);
    }

    // On peut alors consommer
    t->next();

    // Et regarder le suivant
    next = t->next();

    // Vérification du token suivant
    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Column Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    Lexing::Token next_next = t->peek();

    if (next_next.m_Token != Lexing::DOT_T) {
        //  Table et Colonne
        return new ColumnName(tok.m_Value, next.m_Value);
    }

    t->next();

    next_next = t->next();

    // Vérification du token suivant
    if (next_next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected  Column Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Schema + Table + Colonne
    return new ColumnName(tok.m_Value, next.m_Value, next_next.m_Value);
}

std::ostream& operator<<(std::ostream& os, const ColumnName& col)
{
    os << col.Print();

    return os;
}

std::pair<ClauseMember, std::string>
Clause::ParseClauseMember(Lexing::Tokenizer* t)
{

    Lexing::Token next = t->peek();

    ClauseMember member;

    std::string column_used = "";

    switch (next.m_Token) {
    case Lexing::STRING_LITT_T: {
        member = LitteralValue<std::string>(ColumnType::TEXT_C, next.m_Value);

        t->next();

    } break;
    case Lexing::NUM_LITT_T: {
        member = LitteralValue<int>(ColumnType::INTEGER_C, std::stoi(next.m_Value));

        // Passer au suivant ici car Parse un nom de colonne le fait
        t->next();
    } break;
    case Lexing::VAR_NAME_T: {
        column_used = t->peek().m_Value;

        member = *ColumnName::ParseColumnName(t);

    } break;
    default:
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected identifier or value", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    return { member, column_used };
}

std::ostream& operator<<(std::ostream& out, const ClauseMember& member)
{
    if (std::holds_alternative<ColumnName>(member)) {
        out << std::get<ColumnName>(member).getColumnName();
    } else if (std::holds_alternative<LitteralValue<std::string>>(member)) {

        out << std::get<LitteralValue<std::string>>(member).getData();
    } else {
        out << std::get<LitteralValue<int>>(member).getData();
    }

    return out;
}

void Clause::Print(std::ostream& out)
{

    out << "(" << m_Lhs << " " << m_Op << " " << m_Rhs << ") ";
}

Clause* Clause::ParseClause(Lexing::Tokenizer* t)
{
    Lexing::Token next = t->peek();

    std::variant<LogicalOperator, Errors::Error> op;

    auto [lhs, column_used_l] = ParseClauseMember(t);

    op = ParseLogicalOperator(t);

    if (std::holds_alternative<Errors::Error>(op)) {
        throw std::get<Errors::Error>(op);
    }

    next = t->peek();

    auto [rhs, column_used_r] = ParseClauseMember(t);

    auto col_used = std::unordered_set<std::string>();

    col_used.reserve(2);

    if (column_used_l != "")
        col_used.emplace(column_used_l);

    if (column_used_r != "")
        col_used.emplace(column_used_r);

    return new Clause(std::get<LogicalOperator>(op), lhs, rhs, col_used);
}

/*
 * a AND b AND c AND d AND e
 *
 * s = stack<cond>
 *
 *
 * while s non vide:
 *
 *  if next = and  :
 *     clause c2 = ParseClause
 *
 *     s.p
 *
 *
 *  ret BinExpr(c, op, c2)
 * else ret c
 *
 * */

std::unordered_set<std::string> BinaryExpression::MergeColumns(Condition lhs, Condition rhs)
{
    auto res = new std::unordered_set<std::string>();

    std::unordered_set<std::string> left;
    std::unordered_set<std::string> right;

    if (std::holds_alternative<BinaryExpression*>(lhs)) {

        BinaryExpression* b = std::get<BinaryExpression*>(lhs);

        left = b->m_ColumnUsedBelow;
    } else {
        left = std::get<Clause*>(lhs)->m_ColumnUsed;
    }

    if (std::holds_alternative<BinaryExpression*>(rhs)) {

        BinaryExpression* b = std::get<BinaryExpression*>(rhs);

        right = b->m_ColumnUsedBelow;

    } else {
        left = std::get<Clause*>(lhs)->m_ColumnUsed;
    }

    size_t tot_size = left.size() + right.size();

    res->reserve(tot_size);

    for (auto e : left) {
        res->emplace(e);
    }

    for (auto e : right) {
        res->emplace(e);
    }

    return *res;
}

BinaryExpression::Condition BinaryExpression::ParseCondition(Lexing::Tokenizer* t)
{

    Lexing::Token next = t->peek();

    auto arg_pile = Utils::Stack<Condition>();
    auto op_pile = Utils::Stack<LogicalOperator>();

    int parenth_count = 0;
    int nb_count_equal_zero = 0;

    do {
        next = t->peek();

        switch (next.m_Token) {
        case Database::Lexing::TokenType::LPAREN_T: {
            parenth_count++;
            t->next();
        } break;
        case Database::Lexing::TokenType::RPAREN_T: {

            parenth_count--;

            t->next();

            if (parenth_count == 0) {
                nb_count_equal_zero++;
            }

            auto lhs = arg_pile.pop();

            if (!arg_pile.empty()) {
                auto rhs = arg_pile.pop();

                arg_pile.push(new BinaryExpression(op_pile.pop(), lhs, rhs, MergeColumns(lhs, rhs)));

            } else {
                arg_pile.push(lhs);
            }
        } break;
        case Database::Lexing::TokenType::VAR_NAME_T:
        case Database::Lexing::TokenType::STRING_LITT_T:
        case Database::Lexing::TokenType::NUM_LITT_T:
            arg_pile.push(Clause::ParseClause(t));
            break;
        case Database::Lexing::TokenType::OR_T: {
            op_pile.push(LogicalOperator::OR);
            t->next();
        } break;
        case Database::Lexing::TokenType::AND_T: {

            op_pile.push(LogicalOperator::AND);

            t->next();
        } break;
        default: {
            if (!op_pile.empty()) {
                throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected clause after AND/OR", 0, 0, Errors::ERROR_EXPECTED_KEYWORD);
            }

            return arg_pile.pop();
        } break;
        }
    } while (nb_count_equal_zero != 2);

    return arg_pile.pop();
}

void BinaryExpression::PrintCondition(std::ostream& out)
{
    auto s = Utils::Stack<Condition>();

    s.push(this);

    while (!s.empty()) {
        auto n = s.pop();

        if (std::holds_alternative<Clause*>(n)) {
            auto cl = std::get<Clause*>(n);

            cl->Print(out);
        } else {
            auto bexpr = std::get<BinaryExpression*>(n);

            out << (bexpr->m_Op == LogicalOperator::AND ? "AND " : "OR ");

            s.push(bexpr->m_Lhs);
            s.push(bexpr->m_Rhs);
        }
    }

    out << std::endl;;
}
std::unordered_set<std::string> BinaryExpression::ColumnUsedUnderCalcul(std::string TablePrincipale)
{
    std::unordered_set<std::string> left_res = std::visit([&](auto& child) {
        using T = std::decay_t<decltype(child)>;
        if constexpr (std::is_same_v<T, BinaryExpression*>) {
            return child->m_ColumnUsedBelow;
        } else if constexpr (std::is_same_v<T, Clause*>) {
            return child->m_ColumnUsed;
        }
    },
        m_Lhs);

    std::unordered_set<std::string> right_res = std::visit([&](auto& child) {
        using T = std::decay_t<decltype(child)>;
        if constexpr (std::is_same_v<T, BinaryExpression*>) {
            return child->m_ColumnUsedBelow;
        } else if constexpr (std::is_same_v<T, Clause*>) {
            return child->m_ColumnUsed;
        }
    },
        m_Rhs);
    std::unordered_set<std::string> m_ColumnUsedBelow;

    m_ColumnUsedBelow.insert(left_res.begin(), left_res.end());
    m_ColumnUsedBelow.insert(right_res.begin(), right_res.end());

    return m_ColumnUsedBelow;

}

} // namespace parsing
