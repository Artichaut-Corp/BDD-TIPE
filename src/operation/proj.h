#include "../algebrizer_types.h"
#include "../data_process_system/table.h"

#include <memory>
#include <string>
#include <vector>

#ifndef PROJ_H

#define PROJ_H

namespace Database::QueryPlanning {

class Proj {
private:
    std::vector<std::string> m_Cols; // all the column who stays once they got there
    std::string TableNameToExec;

public:
    Proj(std::vector<std::string> cols, std::string Table)
        : TableNameToExec(Table)
        , m_Cols(cols)

    {
    }

    Table Exec(Table table)
    {
        table.Projection(std::make_unique<std::vector<std::string>>(m_Cols));

        return table;
    }

    std::string GetTableName() { return TableNameToExec; }
};

} // Database::QueryPlanning

#endif // !PROJ_H
