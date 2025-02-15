#include "expression.h"
#include <variant>

namespace Compiler::Parsing {

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
                return Errors::Error(Errors::ErrorType::SynxtaxError, "Unexpected '&' in binary expression", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
            }
        }
        // '|' seul est un opérateur arithmétique
        else if (tok.m_Value == "|") {
            if (next.m_Value.front() == '&') {
                t->next();
                return LogicalOperator::AND;
            } else {
                return Errors::Error(Errors::ErrorType::SynxtaxError, "Unexpected '|' in binary expression", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
            }
        }
        // Je sait pas trop ce qu'il fait là
        else {
            return Errors::Error(Errors::ErrorType::SynxtaxError, "Unexpected symbol in binary expression", 0, 0, Errors::ERROR_UNEXPECTED_SYMBOL);
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
        return Errors::Error(Errors::ErrorType::SynxtaxError, "Expected a logical operator in expression", 0, 0, Errors::ERROR_EXPECTED_SYMBOL);
    }
}

Expr* SchemaName::ParseSchemaName(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected Schema, Table or Column name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Regarde le prochain sans le consommer car il est possible que
    // ce soit un nom de schema seul
    Lexing::Token next = t->peek();

    if (next.m_Token != Lexing::DOT_T) {
        // Schema seul
        return new SchemaName(tok.m_Value);
    }

    // On peut alors consommer
    next = t->next();

    // Vérification du token suivant
    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected Schema, Table or Column name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    Lexing::Token next_next = t->peek();

    if (next_next.m_Token != Lexing::DOT_T) {
        // Schema et Table
        return new SchemaName(tok.m_Value, next.m_Value);
    }

    next_next = t->next();

    // Vérification du token suivant
    if (next_next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected Schema, Table or Column name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    return new SchemaName(tok.m_Value, next.m_Value, next_next.m_Value);
}

Expr* TableName::ParseTableName(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected Schema, Table or Column name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Regarde le prochain sans le consommer car il est possible que
    // ce soit un nom de table seul
    Lexing::Token next = t->peek();

    if (next.m_Token != Lexing::DOT_T) {
        return new TableName(tok.m_Value);
    }

    // On peut alors consommer
    next = t->next();

    // Vérification du token suivant
    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected Schema, Table or Column name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Paramètre dans cet ordre car le nom de colonne vient après
    return new TableName(tok.m_Value, next.m_Value);
}

Expr* ColumnName::ParseColumnName(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected Schema, Table or Column name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Comme on parse uniquement un nom de colonne, on retourne directement
    return new ColumnName(tok.m_Value);
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

            // Passer au suivant ici car Parse un nom table ou de schema le fait
            t->next();
        } break;
        case Lexing::VAR_NAME_T: {
            expr = TableName::ParseTableName(t);

        } break;
        default:
            throw Errors::Error(Errors::ErrorType::SynxtaxError, "Expected identifier or value", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
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
