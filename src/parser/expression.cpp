#include "expression.h"
#include "../storage/types.h"
#include "../utils.h"
#include "../utils/unordered_set_utils.h"

#include <cassert>
#include <ostream>
#include <unordered_set>
#include <variant>

namespace Database::Parsing {

std::variant<std::string, Errors::Error> ParseAs(Lexing::Tokenizer* t)
{
    Lexing::Token tok = t->next();

    assert(tok.m_Token == Lexing::AS_T);

    tok = t->next();

    if (tok.m_Token != Lexing::VAR_NAME_T) {
        return Errors::Error(Errors::ErrorType::SyntaxError, "Expected alias name after 'AS'", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    return tok.m_Value;
}

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

    Aliases aka;

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Regarde le prochain sans le consommer car il est possible que
    // ce soit un nom de table seul
    Lexing::Token next = t->peek();

    if (next.m_Token != Lexing::DOT_T) {
        if (next.m_Token == Lexing::AS_T) {
            auto alias = ParseAs(t);

            if (std::holds_alternative<Errors::Error>(alias))
                throw alias;

            aka.insert(std::get<std::string>(alias));
        }


        return new TableName(tok.m_Value, aka);
    }

    // On peut alors consommer le point
    t->next();

    // Et regarder le suivant. On peut le consommer directement
    // car il est certain que l'on doive trouver un nom
    next = t->next();

    // Vérification du token suivant
    if (next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Table Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // On garde la valeur pour explorer la suite
    std::string name = next.m_Value;


    next = t->peek();

    if (next.m_Token == Lexing::AS_T) {
        auto alias = ParseAs(t);

        if (std::holds_alternative<Errors::Error>(alias))
            throw alias;

        aka.insert(std::get<std::string>(alias));
    }

    // Schema + Table
    return new TableName(tok.m_Value, name, aka);
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

    Aliases aka;

    // Seulement les 'variables', c'est à dire chaîne de caractères
    // sans "" ou ''
    if (tok.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected Column Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    // Regarde le prochain sans le consommer car il est possible que
    // ce soit un nom de colonne seul
    Lexing::Token next = t->peek();

    if (next.m_Token != Lexing::DOT_T) {
        if (next.m_Token == Lexing::AS_T) {
            auto alias = ParseAs(t);

            if (std::holds_alternative<Errors::Error>(alias))
                throw alias;

            aka.insert(std::get<std::string>(alias));
        }

        // Schema seul
        return new ColumnName(tok.m_Value, aka);
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

        if (next_next.m_Token == Lexing::AS_T) {
            auto alias = ParseAs(t);

            if (std::holds_alternative<Errors::Error>(alias))
                throw alias;

            aka.insert(std::get<std::string>(alias));
        }

        //  Table et Colonne
        return new ColumnName(next.m_Value, tok.m_Value, aka);
    }

    t->next();

    next_next = t->next();

    // Vérification du token suivant
    if (next_next.m_Token != Lexing::VAR_NAME_T) {
        throw Errors::Error(Errors::ErrorType::SyntaxError, "Expected  Column Name", 0, 0, Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    std::string name = next_next.m_Value;


    next_next = t->peek();

    if (next_next.m_Token == Lexing::AS_T) {
        auto alias = ParseAs(t);

        if (std::holds_alternative<Errors::Error>(alias))
            throw alias;

        aka.insert(std::get<std::string>(alias));
    }

    // Schema + Table + Colonne
    return new ColumnName(tok.m_Value, next.m_Value, next_next.m_Value, aka);
}

std::ostream& operator<<(std::ostream& os, const ColumnName& col)
{
    os << col.Print();

    return os;
}

std::pair<ClauseMember, QueryPlanning::ColonneNamesSet*> Clause::ParseClauseMember(Lexing::Tokenizer* t)
{
    Lexing::Token next = t->peek();
    ClauseMember member;
    QueryPlanning::ColonneNamesSet* column_used = nullptr; // ✅ important

    switch (next.m_Token) {
    case Lexing::STRING_LITT_T: {
        member = Convert::StringToDbString(LitteralValue<std::string>(ColumnType::TEXT_C, next.m_Value).getData());
        t->next();
        break;
    }
    case Lexing::NUM_LITT_T: {
        member = Convert::intToColumnData(LitteralValue<int>(ColumnType::INTEGER_C, std::stoi(next.m_Value)).getData());
        t->next();
        break;
    }
    case Lexing::VAR_NAME_T: {
        auto col_parsed_name = ColumnName::ParseColumnName(t);

        if (col_parsed_name->HaveTable()) {
            auto table = new QueryPlanning::TableNamesSet(col_parsed_name->GetTable());
            column_used = new QueryPlanning::ColonneNamesSet(
                col_parsed_name->getColumnName(),
                *col_parsed_name->GetAlias(),
                table);
        } else {
            column_used = new QueryPlanning::ColonneNamesSet(
                col_parsed_name->getColumnName(),
                *col_parsed_name->GetAlias());
        }

        member = column_used; 
        break;
    }
    default:
        throw Errors::Error(
            Errors::ErrorType::SyntaxError,
            "Expected identifier or value",
            0, 0,
            Errors::ERROR_EXPECTED_IDENTIFIER);
    }

    return { member, column_used }; 
}

std::ostream& operator<<(std::ostream& out, const ClauseMember& member)
{
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(member)) {
        out << member;
    } else if (std::holds_alternative<ColumnData>(member)) {
        auto c = std::get<ColumnData>(member);
        std::visit([&](auto&& elem) {
            using T = std::decay_t<decltype(elem)>;
            if constexpr (std::is_same_v<T, DbString>) {
                out << Convert::DbStringToString(elem);
            } else {
                out << std::to_string(elem);
            }
        },
            c);
        return out;
    }

    return out;
}

void Clause::Print(std::ostream& out)
{

    out << "(";
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(Lhs())) {
        out << *std::get<QueryPlanning::ColonneNamesSet*>(Lhs());
    } else {
        out << Lhs();
    }
    out << " " << m_Op << " ";
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(Rhs())) {
        out << *std::get<QueryPlanning::ColonneNamesSet*>(Rhs());
    } else {
        out << Rhs();
    }
    out << ") ";
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

    auto col_used = new std::unordered_set<QueryPlanning::ColonneNamesSet*>;

    col_used->reserve(2);

    if (column_used_l!= nullptr&& column_used_l->GetMainName() != "")
        col_used->emplace(column_used_l);

    if (column_used_r!= nullptr&&column_used_r->GetMainName() != "")
        col_used->emplace(column_used_r);

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

std::ostream& operator<<(std::ostream& out, const Clause& member)
{
    out << "(" << member.Lhs() << " " << member.Op() << " " << member.Rhs() << ")";
    return out;
}

std::unordered_set<QueryPlanning::ColonneNamesSet*>* BinaryExpression::MergeColumns(Condition lhs, Condition rhs)
{
    auto res = new std::unordered_set<QueryPlanning::ColonneNamesSet*>();

    std::unordered_set<QueryPlanning::ColonneNamesSet*>* left;
    std::unordered_set<QueryPlanning::ColonneNamesSet*>* right;

    if (std::holds_alternative<BinaryExpression*>(lhs)) {

        BinaryExpression* b = std::get<BinaryExpression*>(lhs);

        left = b->m_ColumnUsedBelow;
    } else {
        left = std::get<Clause*>(lhs)->Column();
    }

    if (std::holds_alternative<BinaryExpression*>(rhs)) {

        BinaryExpression* b = std::get<BinaryExpression*>(rhs);

        right = b->m_ColumnUsedBelow;

    } else {
        left = std::get<Clause*>(lhs)->Column();
    }

    size_t tot_size = left->size() + right->size();

    res->reserve(tot_size);

    for (auto e : *left) {
        res->emplace(e);
    }

    for (auto e : *right) {
        res->emplace(e);
    }

    return res;
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

void BinaryExpression::PrintConditionalt(std::ostream& out)
{
    auto n = this;
    auto g = n->m_Lhs;
    auto d = n->m_Rhs;
    out << "(";
    if (std::holds_alternative<Clause*>(g)) {
        auto cl = std::get<Clause*>(g);

        cl->Print(out);
    } else if (std::holds_alternative<BinaryExpression*>(g)) {
        auto bexpr = std::get<BinaryExpression*>(g);
        bexpr->PrintCondition(out);
    } else {
        out << "Condition vide";
    }
    out << ") ";

    out << (n->m_Op == LogicalOperator::AND ? "AND " : "OR ");
    out << "(";

    if (std::holds_alternative<Clause*>(d)) {
        auto cl = std::get<Clause*>(d);

        cl->Print(out);
    } else if (std::holds_alternative<BinaryExpression*>(d)) {
        auto bexpr = std::get<BinaryExpression*>(d);
        bexpr->PrintCondition(out);

    } else {
        out << "Condition vide";
    }
    out << ")";
}
void BinaryExpression::PrintCondition(std::ostream& out)
{
    this->PrintConditionalt(out);
    out << std::endl;
}
BinaryExpression::Condition BinaryExpression::ExtraireCond(std::unordered_set<QueryPlanning::ColonneNamesSet*>* ColonnesAExtraire)
{
    if (BinaryExpression::Op() == LogicalOperator::AND) { // on ne peut pas couper un OR

        std::unordered_set<QueryPlanning::ColonneNamesSet*>* LeftColumn;
        auto left = Lhs();
        if (std::holds_alternative<std::monostate>(left)) {
            LeftColumn = {};
        } else if (std::holds_alternative<Clause*>(left)) {
            LeftColumn = std::get<Clause*>(left)->Column();
        } else {
            LeftColumn = std::get<BinaryExpression*>(left)->Column();
        }
        std::unordered_set<QueryPlanning::ColonneNamesSet*>* RightColumn;
        auto right = Rhs();
        if (std::holds_alternative<std::monostate>(right)) {
            RightColumn = {};
        } else if (std::holds_alternative<Clause*>(right)) {
            RightColumn = std::get<Clause*>(right)->Column();
        } else {
            RightColumn = std::get<BinaryExpression*>(right)->Column();
        }

        if (Utils::is_subset(LeftColumn, ColonnesAExtraire)) { // je peut prendre tout gauche
            // il faut tester si on ne peut pas avoir des truc à droite
            if (Utils::is_subset(RightColumn, ColonnesAExtraire)) { // on peut tout prendre à droite et à gauche
                NullifyLhs();
                NullifyRhs();
                return new BinaryExpression(LogicalOperator::AND, left, right, MergeColumns(left, right));
            } else {
                BinaryExpression::Condition RecupADroite;
                if (std::holds_alternative<std::monostate>(right)) {
                    RecupADroite = std::monostate {};
                } else if (std::holds_alternative<Clause*>(right)) {
                    if (Utils::is_subset(std::get<Clause*>(right)->Column(), ColonnesAExtraire)) { // techniqument impossible, car on serais allé dans le cas où on peut tout prendre à droite
                        NullifyRhs();
                        RecupADroite = right;
                    } else {
                        RecupADroite = std::monostate {}; // on ne peut pas découper une clause, donc on renvoie rien
                    }
                } else {
                    RecupADroite = std::get<BinaryExpression*>(right)->ExtraireCond(ColonnesAExtraire);
                }

                bool RecupADroiteEstVide = IsEmpty(RecupADroite);

                auto temp = left;
                NullifyLhs();
                if (RecupADroiteEstVide) {
                    return temp; // on a rien trouvé à droite, donc on renvoie juste tout gauche
                } else { // y'as des truc à droite donc on les regroupe et on renvoie ça

                    return new BinaryExpression(LogicalOperator::AND, RecupADroite, temp, MergeColumns(RecupADroite, temp));
                }
            }
        } else { // on ne peut pas tout prendre à gauche, donc on teste à droite et on prend un max à gauche
            BinaryExpression::Condition RecupAGauche;
            if (std::holds_alternative<std::monostate>(left)) {
                RecupAGauche = std::monostate {};
            } else if (std::holds_alternative<Clause*>(left)) {
                if (Utils::is_subset(std::get<Clause*>(left)->Column(), ColonnesAExtraire)) { // techniqument impossible, car on serais allé dans le cas où on peut tout prendre à droite
                    NullifyRhs();
                    RecupAGauche = left;
                } else {
                    RecupAGauche = std::monostate {}; // on ne peut pas découper une clause, donc on renvoie rien
                }
            } else {
                RecupAGauche = std::get<BinaryExpression*>(left)->ExtraireCond(ColonnesAExtraire);
            }

            bool RecupAGaucheEstVide = IsEmpty(RecupAGauche);

            if (Utils::is_subset(RightColumn, ColonnesAExtraire)) { // je peut tout prendre à droite
                auto temp = right;
                NullifyRhs();
                // il faut tester si on n'as pas eu des truc à gauche
                if (RecupAGaucheEstVide) {
                    return temp; // on a rien trouvé à gauche, donc on renvoie juste tout droite
                } else { // y'as des truc à gauche donc on les regroupe et on renvoie ça
                    return new BinaryExpression(LogicalOperator::AND, RecupAGauche, temp, MergeColumns(RecupAGauche, temp));
                }
            } else { // On ne peut pas tout prendre à droite ni tout prendre à gauche
                BinaryExpression::Condition RecupADroite;
                if (std::holds_alternative<std::monostate>(right)) {
                    RecupADroite = std::monostate {};
                } else if (std::holds_alternative<Clause*>(right)) {
                    if (Utils::is_subset(std::get<Clause*>(right)->Column(), ColonnesAExtraire)) { // techniqument impossible, car on serais allé dans le cas où on peut tout prendre à droite
                        NullifyRhs();
                        RecupADroite = right;
                    } else {
                        RecupADroite = std::monostate {}; // on ne peut pas découper une clause, donc on renvoie rien
                    }
                } else {
                    RecupADroite = std::get<BinaryExpression*>(right)->ExtraireCond(ColonnesAExtraire);
                }

                bool RecupADroiteEstVide = IsEmpty(RecupADroite);

                if (RecupADroiteEstVide) {
                    if (RecupAGaucheEstVide) {
                        return std::monostate {};
                    } else {
                        return RecupAGauche;
                    }
                } else {
                    if (RecupAGaucheEstVide) {
                        return RecupADroite;
                    } else {
                        auto temp = new BinaryExpression(LogicalOperator::AND, RecupADroite, RecupAGauche, MergeColumns(RecupADroite, RecupAGauche));
                        return temp;
                        ;
                    }
                }
            }
        }
    } else {
        return std::monostate {};
    }
}

bool Clause::Eval(std::unordered_map<std::string, ColumnData*>* CombinaisonATester)
{
    ColumnData LeftVal;
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(Lhs())) {
        auto temp = std::get<QueryPlanning::ColonneNamesSet*>(Lhs());
        LeftVal = *(*CombinaisonATester)[temp->GetMainName()];

    } else if (std::holds_alternative<ColumnData>(Lhs())) {
        LeftVal = std::get<ColumnData>(Lhs());
    } else {
        throw Errors::Error(Errors::ErrorType::RuntimeError,
            "Unknown type in the Clause parameter",
            0, 0, Errors::ERROR_UNKNOW_TYPE_BINARYEXPR);
    }

    ColumnData RightVal;
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(Rhs())) {
        auto temp = std::get<QueryPlanning::ColonneNamesSet*>(Rhs());
        RightVal = *(*CombinaisonATester)[temp->GetMainName()];

    } else if (std::holds_alternative<ColumnData>(Rhs())) {

        RightVal = std::get<ColumnData>(Rhs());
    } else {
        throw Errors::Error(Errors::ErrorType::RuntimeError,
            "Unknown type in the Clause parameter",
            0, 0, Errors::ERROR_UNKNOW_TYPE_BINARYEXPR);
    }
    switch (Op()) {
    case Parsing::LogicalOperator::EQ:
        return Database::QueryPlanning::operator==(LeftVal, RightVal);
    case Parsing::LogicalOperator::GT:
        return Database::QueryPlanning::operator>(LeftVal, RightVal);

    case Parsing::LogicalOperator::LT:
        return Database::QueryPlanning::operator<(LeftVal, RightVal);
    case Parsing::LogicalOperator::GTE:
        return Database::QueryPlanning::operator>=(LeftVal, RightVal);
    case Parsing::LogicalOperator::LTE:
        return Database::QueryPlanning::operator<=(LeftVal, RightVal);
    case Parsing::LogicalOperator::NE:
        return Database::QueryPlanning::operator!=(LeftVal, RightVal);
    default:
        throw Errors::Error(Errors::ErrorType::RuntimeError,
            "Unknown Logical Operator",
            0, 0, Errors::ERROR_UNKNOW_LOGICAL_OPERATOR);
    }
}

bool BinaryExpression::Eval(std::unordered_map<std::string, ColumnData*>* CombinaisonATester)
{
    bool ResultAGauche;
    auto left = Lhs();
    if (std::holds_alternative<BinaryExpression*>(left)) {
        ResultAGauche = std::get<BinaryExpression*>(left)->Eval(CombinaisonATester);
    } else if (std::holds_alternative<Clause*>(left)) {
        ResultAGauche = std::get<Clause*>(left)->Eval(CombinaisonATester);
    } else {
        ResultAGauche = true;
    }
    if (Op() == LogicalOperator::OR && ResultAGauche) {
        return true; // on rend paraisseuse l'évaluation
    } else if (m_Op == LogicalOperator::AND && !ResultAGauche) {
        return false;
    } else {
        bool ResultADroite;
        auto droite = Rhs();
        if (std::holds_alternative<BinaryExpression*>(droite)) {
            ResultADroite = std::get<BinaryExpression*>(droite)->Eval(CombinaisonATester);
        } else if (std::holds_alternative<Clause*>(droite)) {
            ResultADroite = std::get<Clause*>(droite)->Eval(CombinaisonATester);
        } else {
            ResultADroite = true;
        }
        if (Op() == LogicalOperator::OR) {
            return ResultADroite;
        } else {
            return ResultADroite && ResultAGauche;
        }
    }
}

void Clause::FormatColumnName(QueryPlanning::TableNamesSet* NomTablePrincipale)
{
    m_ColumnUsed->clear();
    auto left = Lhs();
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(left)) {
        auto LeftColumn = std::get<QueryPlanning::ColonneNamesSet*>(left);
        if (!LeftColumn->HaveTableSet()) {
            LeftColumn->SetTableSet(NomTablePrincipale);
        }
        this->EditLhs(left);
        m_ColumnUsed->insert(LeftColumn);
    }

    auto right = Rhs();
    if (std::holds_alternative<QueryPlanning::ColonneNamesSet*>(right)) {
        auto RightColumn = std::get<QueryPlanning::ColonneNamesSet*>(right);
        if (!RightColumn->HaveTableSet()) {
            RightColumn->SetTableSet(NomTablePrincipale);
        }
        this->EditRhs(right);
        m_ColumnUsed->insert(RightColumn);
    }
}

void BinaryExpression::FormatColumnName(QueryPlanning::TableNamesSet* NomTablePrincipale)
{
    m_ColumnUsedBelow = {};
    auto left = Lhs();
    if (std::holds_alternative<BinaryExpression*>(left)) {
        std::get<BinaryExpression*>(left)->FormatColumnName(NomTablePrincipale);
        m_ColumnUsedBelow->insert(std::get<BinaryExpression*>(left)->Column()->begin(), std::get<BinaryExpression*>(left)->Column()->end());
    } else if (std::holds_alternative<Clause*>(left)) {
        std::get<Clause*>(left)->FormatColumnName(NomTablePrincipale);
        m_ColumnUsedBelow->insert(std::get<Clause*>(left)->Column()->begin(), std::get<Clause*>(left)->Column()->end());
    }
    auto right = Rhs();
    if (std::holds_alternative<BinaryExpression*>(right)) {
        std::get<BinaryExpression*>(right)->FormatColumnName(NomTablePrincipale);
        m_ColumnUsedBelow->insert(std::get<BinaryExpression*>(right)->Column()->begin(), std::get<BinaryExpression*>(right)->Column()->end());

    } else if (std::holds_alternative<Clause*>(right)) {
        std::get<Clause*>(right)->FormatColumnName(NomTablePrincipale);
        m_ColumnUsedBelow->insert(std::get<Clause*>(right)->Column()->begin(), std::get<Clause*>(right)->Column()->end());
    }
}

} // namespace parsing
