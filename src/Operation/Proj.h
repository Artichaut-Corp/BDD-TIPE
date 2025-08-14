#include "../Data Process System/Table.h"
#include "../algebrizer/type def.h"
#include "pred.h"
#include <string>
#include <vector>
using namespace Database::Querying;
class Proj {
private:
    ;
    std::vector<std::string> m_Cols;  //list of all the column who are being checked
    std::vector<Predicat_list> m_Conds;  // the list of condition those column are being test on, m_cols[x] is tested on m_comps[x]
    Table m_Table;

public:
    Proj(std::vector<std::string> cols, std::vector<Predicat_list> cond, Table table)
        : m_Cols(cols)
        , m_Conds(cond)
        , m_Table(table)

    {
    };

    Table Exec()
    {
        m_Table.Projection(m_Cols, m_Conds);
        return  m_Table;
    }
    void edit_table(Table new_table)
    {
        m_Table = new_table;
    }
};
