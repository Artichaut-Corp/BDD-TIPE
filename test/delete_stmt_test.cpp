#include <gtest/gtest.h>

#include "../src/parser/parser.h"

using namespace Compiler::Parsing;

// DELETE FROM city;
TEST(DeleteStmtTest, BasicDelete)
{
    auto table = new TableName("city");

    auto del = new DeleteStmt(table, nullptr);

    Node delete_stmt = Node(del);
}

// DELETE FROM city WHERE name='Vannes';
TEST(DeleteStmtTest, DeleteWithCond)
{
    auto left = new ColumnName("name");

    auto right = new LitteralValue<std::string>(ColumnType::TEXT_C, "Vannes");

    auto condition = new BinaryExpression(LogicalOperator::EQ, left, right);

    auto where = new WhereClause(condition);

    auto table = new TableName("city");

    auto del = new DeleteStmt(table, where);

    Node delete_stmt = Node(del);
}
