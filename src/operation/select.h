#include "../algebrizer_types.h"
#include "../data_process_system/table.h"

#include "pred.h"

#include <memory>
#include <string>
#include <vector>

#ifndef SELEC_H
#define SELEC_H
namespace Database::QueryPlanning {
class Select {
private:
    ;
    std::vector<std::string> m_Cols; // list of all the column who are being checked
    std::vector<std::shared_ptr<Predicat_list>> m_Conds; // the list of condition those column are being test on, m_cols[x] is tested on m_comps[x]
    std::string TableNameToExec;

public:
    Select(std::vector<std::string> cols, std::vector<std::shared_ptr<Predicat_list>> cond)
        : m_Cols(cols)
        , m_Conds(cond)

    {
        TableNameToExec = "country";

    };

    Table* Exec(Table* table)
    {
        table->Selection(std::make_shared<std::vector<std::shared_ptr<Predicat_list>>>(m_Conds), std::make_shared<std::vector<std::string>>(m_Cols));
        return table;
    }
    std::string GetTableName(){return TableNameToExec;}

};

} // Database::QueryPlanning

#endif // !SELEC_H
