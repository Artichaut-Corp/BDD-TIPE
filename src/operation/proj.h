#include "../algebrizer_types.h"
#include "../data_process_system/table.h"

#include <memory>
#include <unordered_set>

#ifndef PROJ_H

#define PROJ_H

namespace Database::QueryPlanning {

class Proj {
private:
    std::unordered_set<ColonneNamesSet*>* m_Cols; // all the column who stays once they got there
    TableNamesSet* TableNameToExec;

public:
    Proj(std::unordered_set<ColonneNamesSet*>* cols, TableNamesSet* Table)
        : TableNameToExec(Table)
        , m_Cols(cols)

    {
    }

    Table* Exec(Table* table)
    {
        table->Projection(std::make_unique<std::unordered_set<ColonneNamesSet*>>(*m_Cols));

        return table;
    }
    TableNamesSet* GetTableName() { return TableNameToExec; }

    std::unordered_set<ColonneNamesSet*>* Getm_Cols(){return m_Cols;}
};

} // Database::QueryPlanning

#endif // !PROJ_H
