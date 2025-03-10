#include "expression.h"
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

BinaryExpression* BinaryExpression::ParseBinaryExpression(Lexing::Tokenizer* t)
{

    Lexing::Token next = t->peek();
    std::variant<LogicalOperator, Errors::Error> op;

    Expr* expr;

    do {
        switch (next.m_Token) {
        case Lexing::STRING_LITT_T: {
            expr = new LitteralValue<std::string>(ColumnType::TEXT_C, next.m_Value);

            t->next();

        } break;
        case Lexing::NUM_LITT_T: {
            expr = new LitteralValue<int>(ColumnType::INTEGER_C, std::stoi(next.m_Value));

            // Passer au suivant ici car Parse un nom de colonne le fait
            t->next();
        } break;
        case Lexing::VAR_NAME_T: {
            expr = ColumnName::ParseColumnName(t);

        } break;
        default:
            throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected identifier or value", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
        }

        // Modif ca. Dans le cas ou c'est un point virgule stop
        if (t->peek().m_Token == Lexing::SEMI_COLON_T) {
            return new BinaryExpression(expr);
        }
        op = ParseLogicalOperator(t);

        if (std::holds_alternative<Errors::Error>(op)) {
            throw std::get<Errors::Error>(op);
        }

        next = t->peek();

    } while (std::holds_alternative<LogicalOperator>(op));

    return new BinaryExpression(ParseBinaryExpression(t), std::get<LogicalOperator>(op), expr);
}

} // namespace parsing
