#include "../Data Process System/Table.h"
#include "../algebrizer/type def.h"
#include "pred.h"
#include <string>
#include <vector>
using namespace Database::Querying;
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
        m_Table->Selection(m_Cols);
        return m_Table
    }

    void edit_table(Table new_table)
    {
        m_Table = new_table;
    }
};
