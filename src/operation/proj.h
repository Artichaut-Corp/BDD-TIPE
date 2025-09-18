#include "../algebrizer_types.h"
#include "../data_process_system/table.h"

#include "pred.h"

#include <memory>
#include <string>
#include <vector>

#ifndef PROJ_H

namespace Database::QueryPlanning {
class Proj {
private:
    ;
    std::vector<std::string> m_Cols; // list of all the column who are being checked
    std::vector<std::unique_ptr<Predicat_list>> m_Conds; // the list of condition those column are being test on, m_cols[x] is tested on m_comps[x]

public:
    Proj(std::vector<std::string> cols, std::vector<std::unique_ptr<Predicat_list>> cond)
        : m_Cols(cols)
        , m_Conds(cond)

    {
    };

    std::unique_ptr<Table> Exec(std::unique_ptr<Table> table)
    {
        table->Projection(std::make_shared<std::vector<std::unique_ptr<Predicat_list>>>(m_Conds), std::make_shared<std::vector<std::string>>(m_Cols));
        return table;
    }
};

} // Database::QueryPlanning

#endif // !PROJ_H
