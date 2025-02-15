#include <gtest/gtest.h>
#include <variant>

#include "../src/parser/parser.h"

using namespace Compiler::Parsing;

// DELETE FROM ville;
TEST(DeleteStmtTest, BasicDelete)
{
    auto table = new TableName("city");

    auto del = new DeleteStmt(table, nullptr);

    Node delete_stmt = Node(del);
}

/*
TEST(DeleteStmtTest, ParseBasicDelete)
{
    auto p = Parser("DELETE FROM ville;");

    std::variant<Statement*, Compiler::Errors::Error> n = p.Parse();

    if (std::holds_alternative<Statement*>(n)) {

        Statement* d = std::get<Statement*>(n);

        DeleteStmt* e = std::get<DeleteStmt*>(*d);

    }

    else {
        ASSERT_EQ(1, 2);
    }
}
*/

// DELETE FROM ville WHERE name='Vannes';
TEST(DeleteStmtTest, DeleteWithCond)
{
    auto left = new ColumnName("name");

    auto right = new LitteralValue<std::string>(ColumnType::TEXT_C, "Vannes");

    auto condition = new BinaryExpression(left, LogicalOperator::EQ, right);

    auto where = new WhereClause(condition);

    auto table = new TableName("city");

    auto del = new DeleteStmt(table, where);

    Node delete_stmt = Node(del);
}
