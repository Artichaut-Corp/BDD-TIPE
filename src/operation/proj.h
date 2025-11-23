#include "../algebrizer_types.h"
#include "../data_process_system/meta-table.h"

#include <memory>
#include <unordered_set>

#ifndef PROJ_H

#define PROJ_H

namespace Database::QueryPlanning {

class Proj {
private:
    std::unordered_set<std::shared_ptr<ColonneNamesSet>>* m_Cols; // all the column who stays once they got there
    std::shared_ptr<TableNamesSet> TableNameToExec;

public:
    Proj(std::unordered_set<std::shared_ptr<ColonneNamesSet>>* cols, std::shared_ptr<TableNamesSet> Table)
        : TableNameToExec(Table)
        , m_Cols(cols)

    {
    }

    std::shared_ptr<MetaTable> Exec(std::shared_ptr<MetaTable> table)
    {
        auto temp = std::make_unique<std::unordered_set<std::shared_ptr<ColonneNamesSet>>>(*m_Cols);
        table->Projection(std::move(temp));

        return table;
    }
    std::shared_ptr<TableNamesSet> GetTableName() { return TableNameToExec; }

    std::unordered_set<std::shared_ptr<ColonneNamesSet>>* Getm_Cols() { return m_Cols; }
};

} // Database::QueryPlanning

#endif // !PROJ_H
