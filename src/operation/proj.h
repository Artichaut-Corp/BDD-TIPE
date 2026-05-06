#include "algebrizer_types.h"
#include "data_process_system/meta-table.h"

#include <memory>
#include <unordered_set>

#ifndef PROJ_H

#define PROJ_H

namespace Database::QueryPlanning {

class Proj {
private:
    // all the column who stays once they got there
    std::unique_ptr<std::unordered_set<const ColonneNamesSet*>> m_Cols;
    const TableNamesSet& TableNameToExec;

public:
    Proj(std::unique_ptr<std::unordered_set<const ColonneNamesSet*>> cols, const TableNamesSet& Table)
        : TableNameToExec(Table)
        , m_Cols(std::move(cols))

    {
    }

    MetaTable* Exec(MetaTable* table)
    {
        auto temp = std::make_unique<std::unordered_set<const ColonneNamesSet*>>(*m_Cols);

        table->Projection(std::move(temp));

        return table;
    }

    const TableNamesSet& GetTableName() { return TableNameToExec; }

    std::unordered_set<const ColonneNamesSet*>& GetCols() { return *m_Cols.get(); }
};

} // Database::QueryPlanning

#endif // !PROJ_H
