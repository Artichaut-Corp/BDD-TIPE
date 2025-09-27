#include "../parser/dml.h"
#include "../algebrizer/tree.h"
#include "../data_process_system/racine.h"
#include "../operation/pred.h"
#include "../operation/join.h"
#include "../operation/agreg.h"
#include "../database.h"

#ifndef ALGEBRIZER_H

#define ALGEBRIZER_H

namespace Database::QueryPlanning {

std::string GetColumnFullName(const std::string& NomTablePrincipale, Database::Parsing::ColumnName* Colonne);

void ConversionEnArbre_ET_excution(Database::Parsing::SelectStmt* Selection,Storing::File * File,std::unordered_map<std::basic_string<char>, Database::Storing::TableInfo> * IndexGet);

};
#endif // ! ALGEBRIZER_H  ALGEBRIZER_H