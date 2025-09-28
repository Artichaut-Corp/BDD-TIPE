#include <gtest/gtest.h>
#include <variant>

#include "../src/parser.h"

using namespace Database::Parsing;

// Sur une table exemple de la forme:
// Ville {
//  Text Nom,
//  Int  Habitants,
//  Text Region
// }
//

// SELECT * FROM Ville;
TEST(SelectStmtTest, BasicSelectAll)
{

    Parser* parser = new Parser("SELECT * FROM ville;");

    auto n = parser->Parse();

    ASSERT_TRUE(std::holds_alternative<Statement>(n));

    auto stmt = std::get<Statement>(n);

    ASSERT_TRUE(std::holds_alternative<SelectStmt*>(stmt));

    auto select = std::get<SelectStmt*>(stmt);
}

// SELECT Nom, Habitants FROM Ville;
TEST(SelectStmtTest, BasicSelect)
{
    Parser* parser = new Parser("SELECT nom, pop FROM ville;");

    auto n = parser->Parse();

    ASSERT_TRUE(std::holds_alternative<Statement>(n));
    auto stmt = std::get<Statement>(n);

    ASSERT_TRUE(std::holds_alternative<SelectStmt*>(stmt));
    auto select = std::get<SelectStmt*>(stmt);
}

// SELECT DISTINCT Nom FROM Ville;
TEST(SelectStmtTest, BasicSelectDistinct) { }

// SELECT * FROM Ville WHERE Habitants > 50000;
TEST(SelectStmtTest, SelectWithCond)
{
    Parser* parser = new Parser("SELECT *  FROM ville WHERE pop > 10;");

    auto n = parser->Parse();

    ASSERT_TRUE(std::holds_alternative<Statement>(n));

    auto stmt = std::get<Statement>(n);

    ASSERT_TRUE(std::holds_alternative<SelectStmt*>(stmt));

    auto select = std::get<SelectStmt*>(stmt);

    BinaryExpression* cond;

    /*
        ASSERT_NO_THROW(
            cond = select->getWhere()->getCondition(););

        ASSERT_EQ(cond->Op().value(), LogicalOperator::GT);

        ColumnName* lhs;

        ASSERT_NO_THROW(
            lhs = std::get<ColumnName*>(cond->m_Lhs));

        LitteralValue<int>* rhs;

        ASSERT_NO_THROW(
            rhs = std::get<LitteralValue<int>*>(cond->m_Rhs));

        ASSERT_EQ(lhs->getColumnName(), "pop");

        ASSERT_EQ(rhs->getData(), 10);
        */
}

// SELECT * FROM Ville ORDER BY Habitants DESC;
TEST(SelectStmtTest, SelectOrdered) { }

// TODO: Voir les joins ici
