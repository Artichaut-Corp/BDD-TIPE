#include "../algebrizer_types.h"
#include "../data_process_system/meta-table.h"
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
    std::shared_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> m_Cols; // set of all the column who are being checked
    Parsing::BinaryExpression::Condition m_Conds; // the condition those column are being test on
    std::shared_ptr<TableNamesSet> TableNameToExec;

public:
    Select(std::shared_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> cols, Parsing::BinaryExpression::Condition cond, std::shared_ptr<TableNamesSet> Table)
        : TableNameToExec(Table)
        , m_Cols(std::move(cols))
        , m_Conds(cond)

    {
    };
    Parsing::BinaryExpression::Condition GetCond() { return m_Conds; }

    std::shared_ptr<MetaTable> Exec(std::shared_ptr<MetaTable> table)
    {
        if (std::holds_alternative<std::monostate>(m_Conds)) {
            return table; // pas besoin dans le reflechir la comparaison est nulle
        }
        table->Selection(m_Conds, m_Cols);
        return table;
    }
    std::shared_ptr<TableNamesSet> GetTableName()
    {
        return TableNameToExec;
    }

    void NullifyCond()
    {
        m_Cols = nullptr;
        m_Conds = std::monostate {};
    }
    std::shared_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> Getm_Cols()
    {
        return m_Cols;
    }
};

} // Database::QueryPlanning

#endif // !SELEC_H
