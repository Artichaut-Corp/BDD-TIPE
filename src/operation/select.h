#include "../algebrizer_types.h"
#include "../data_process_system/table.h"

#include "pred.h"

#include <string>
#include <vector>

#ifndef SELECT_H

#define SELECT_H

namespace Database::QueryPlanning {

class Select {
private:
    ;
    std::vector<std::string> m_Cols; // all the column who need to be deleted once they got there
    Table m_Table;

public:
    Select(std::vector<std::string> cols, Table table)
        : m_Cols(cols)
        , m_Table(table)

    {
    }

    Table Exec()
    {
        m_Table.Selection(m_Cols);

        return m_Table;
    }

    void edit_table(Table new_table)
    {
        m_Table = new_table;
    }
};

} // Database::QueryPlanning

#endif // !SELECT_H
