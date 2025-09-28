#include "../algebrizer_types.h"
#include "../data_process_system/table.h"
#include "../parser/expression.h"
#include "pred.h"

#include <memory>
#include <string>
#include <unordered_set>

#ifndef SELEC_H
#define SELEC_H
namespace Database::QueryPlanning {
class Select {
private:
    ;
    std::unique_ptr<std::unordered_set<std::string>> m_Cols; // set of all the column who are being checked
    Parsing::BinaryExpression::Condition m_Conds; // the condition those column are being test on
    std::string TableNameToExec;
    std::string TablePrincipale;

public:
    Select(std::unique_ptr<std::unordered_set<std::string>> cols,Parsing::BinaryExpression::Condition cond,std::string TablePrincipale_)
        : m_Cols(std::move(cols))
        , m_Conds(cond)
        ,TablePrincipale(TablePrincipale_)

    {
        TableNameToExec = "city";

    };

    Table* Exec(Table* table)
    {
        table->Selection(m_Conds, std::move(m_Cols), TablePrincipale);
        return table;
    }
    std::string GetTableName(){return TableNameToExec;}

};

} // Database::QueryPlanning

#endif // !SELEC_H
