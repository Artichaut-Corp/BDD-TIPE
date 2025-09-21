#include "../parser/dml.h"
#include "../algebrizer/tree.h"
#include "../data_process_system/racine.h"
#include "../operation/pred.h"
#include "../operation/join.h"
#include "../operation/agreg.h"
#ifndef ALGEBRIZER_H

#define ALGEBRIZER_H

namespace Database::QueryPlanning {

std::string GetColumnFullName(std::string NomTablePrincipale, Database::Parsing::ColumnName* Colonne);

std::unique_ptr<Node> ConversionEnArbre(std::unique_ptr<Database::Parsing::SelectStmt> Selection);

};
#endif // ! ALGEBRIZER_H  ALGEBRIZER_H