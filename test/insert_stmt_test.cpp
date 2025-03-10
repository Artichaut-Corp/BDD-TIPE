#include <gtest/gtest.h>

#include "../src/parser.h"

using namespace Database::Parsing;

// Sur une table exemple de la forme:
// Ville {
//  Text Nom,
//  Int  Habitants,
//  Text Region
// }

// INSERT INTO Ville DEFAULT VALUES;
TEST(InsertStmtTest, DefaultCase)
{
    auto name = new TableName("Ville");

    auto insert = new InsertStmt(name);

    auto node = Node<InsertStmt>(insert);
}

// INSERT INTO Ville VALUES ("Vannes",  54420, "Bretagne");
TEST(InsertStmtTest, SimpleValues)
{
    auto data = std::vector<Expr>(3);

    data[0] = LitteralValue<std::string>(ColumnType::TEXT_C, "Vannes");
    data[1] = LitteralValue<int>(ColumnType::INTEGER_C, 54420);
    data[2] = LitteralValue<std::string>(ColumnType::TEXT_C, "Bretagne");

    auto dataOfData = std::vector<std::vector<Expr>>(1);

    dataOfData[0] = data;

    auto name = new TableName("Ville");

    auto insert = new InsertStmt(name, false, dataOfData);

    auto node = Node<InsertStmt>(insert);
}

// INSERT INTO Ville (Habitants, Nom, Region) VALUES (54420, "Vannes", "Bretagne"), (325070, "Nantes", "Bretagne");
TEST(InsertStmtTest, SimpleValuesWithDefinedOrder)
{

    auto order = std::vector<ColumnName>(3);

    order[0] = ColumnName("Habitants");
    order[1] = ColumnName("Nom");
    order[2] = ColumnName("Region");

    auto data1 = std::vector<Expr>(3);

    data1[0] = LitteralValue<int>(ColumnType::INTEGER_C, 54420);
    data1[1] = LitteralValue<std::string>(ColumnType::TEXT_C, "Vannes");
    data1[2] = LitteralValue<std::string>(ColumnType::TEXT_C, "Bretagne");

    auto data2 = std::vector<Expr>(3);

    data2[0] = LitteralValue<int>(ColumnType::INTEGER_C, 325070);
    data2[1] = LitteralValue<std::string>(ColumnType::TEXT_C, "Nantes");
    data2[2] = LitteralValue<std::string>(ColumnType::TEXT_C, "Bretagne");

    auto dataOfData = std::vector<std::vector<Expr>>(2);

    dataOfData[0] = data1;
    dataOfData[1] = data2;

    auto name = new TableName("Ville");

    auto insert = new InsertStmt(name, false, order, dataOfData);

    auto node = Node<InsertStmt>(insert);
}
