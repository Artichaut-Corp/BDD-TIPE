#include <gtest/gtest.h>

#include "../src/parser.h"

using namespace Database::Parsing;

// Sur une table exemple de la forme:
// Ville {
//  Text Nom,
//  Int  Habitants,
//  Text Region
// }

// SELECT * FROM Ville;
TEST(SelectStmtTest, BasicSelectAll) { }

// SELECT Nom, Habitants FROM Ville;
TEST(SelectStmtTest, BasicSelect) { }

// SELECT DISTINCT Nom FROM Ville;
TEST(SelectStmtTest, BasicSelectDistinct) { }

// SELECT * FROM Ville WHERE Habitants > 50000;
TEST(SelectStmtTest, SelectWithCond) { }

// SELECT * FROM Ville ORDER BY Habitants DESC;
TEST(SelectStmtTest, SelectOrdered) { }

// TODO: Voir les joins ici
