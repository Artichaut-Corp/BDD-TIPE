#include "algebrizer/tree.h"
#include "data_process_system/namingsystem.h"
#include "data_process_system/racine.h"
#include "database.h"
#include "operation/agreg.h"
#include "operation/join.h"
#include "operation/pred.h"
#include "parser/dml.h"


#ifndef ALGEBRIZER_H

#define ALGEBRIZER_H

namespace Database::QueryPlanning {

ColonneNamesSet* ConvertToStandardColumnName(const TableNamesSet& NomTablePrincipale, Database::Parsing::ColumnName* Colonne, std::unordered_map<std::string, TableNamesSet*>* variation_of_tablename_to_main_table_name);

TableNamesSet* ConvertToStandardTableName(Database::Parsing::TableName* Table, std::unordered_map<std::string, TableNamesSet*>* variation_of_tablename_to_main_table_name);

void ConversionEnArbre_ET_excution(Database::Parsing::SelectStmt* Selection, Storing::File* File, std::unordered_map<std::string, Database::Storing::TableInfo>* IndexGet, DatabaseSetting* params);

};

#endif // ! ALGEBRIZER_H
