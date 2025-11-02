#include "../algebrizer_types.h"
#include "../data_process_system/table.h"
#include "../parser/expression.h"
#include "pred.h"

#include <memory>
#include <unordered_set>
#include <variant>

#ifndef SELEC_H
#define SELEC_H
namespace Database::QueryPlanning {
class Select {
private:
    ;
    std::unique_ptr<std::unordered_set<ColonneNamesSet*>> m_Cols; // set of all the column who are being checked
    Parsing::BinaryExpression::Condition m_Conds; // the condition those column are being test on
    TableNamesSet* TableNameToExec;

public:
    Select(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> cols, Parsing::BinaryExpression::Condition cond, TableNamesSet* Table)
        : TableNameToExec(Table)
        , m_Cols(std::move(cols))
        , m_Conds(cond)

    {
    };
    Parsing::BinaryExpression::Condition GetCond() { return m_Conds; }

    Table* Exec(Table* table)
    {
        if (std::holds_alternative<std::monostate>(m_Conds)) {
            return table; // pas besoin dans le reflechir la comparaison est nulle
        }
        table->Selection(m_Conds, std::move(m_Cols));
        return table;
    }
    TableNamesSet* GetTableName()
    {
        return TableNameToExec;
    }

    void NullifyCond()
    {
        m_Cols = nullptr;
        m_Conds = std::monostate {};
    }
};

} // Database::QueryPlanning

#endif // !SELEC_H
