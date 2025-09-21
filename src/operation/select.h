#include "../algebrizer_types.h"
#include "../data_process_system/table.h"

#include "pred.h"

#include <memory>
#include <string>
#include <vector>

#ifndef SELECT_H

#define SELECT_H

namespace Database::QueryPlanning {

class Select {
private:
    std::vector<std::string> m_Cols; // all the column who stays once they got there
    
public:
    Select(std::vector<std::string> cols)
        : m_Cols(cols)

    {
    }

    Table* Exec(Table* table)
    {
        table->Selection(std::make_shared<std::vector<std::string>> (m_Cols));

        return table;
    }
};

} // Database::QueryPlanning

#endif // !SELECT_H
