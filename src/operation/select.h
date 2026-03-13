#include "algebrizer_types.h"
#include "data_process_system/meta-table.h"
#include "parser/expression.h"

#include "pred.h"

#include <cassert>
#include <functional>
#include <memory>
#include <unordered_set>
#include <variant>

#ifndef SELEC_H
#define SELEC_H

namespace Database::QueryPlanning {

class Select {

private:
    const TableNamesSet& TableNameToExec;

    // the condition those column are being test on
    // UUPO
    std::reference_wrapper<Parsing::BinaryExpression::Condition> m_Conds;

    // set of all the column who are being checked
    std::unique_ptr<std::unordered_set<ColonneNamesSet*>> m_Cols;

public:
    Select(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> cols, Parsing::BinaryExpression::Condition cond, const TableNamesSet& Table)
        : TableNameToExec(Table)
        , m_Cols(std::move(cols))
        , m_Conds(cond)

    {
    };
    Select(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> cols, Parsing::BinaryExpression::Condition* cond, const TableNamesSet& Table)
        : TableNameToExec(Table)
        , m_Cols(std::move(cols))
        , m_Conds(*cond)

    {
    };
    const std::reference_wrapper<Parsing::BinaryExpression::Condition> GetCond() const  { return m_Conds; }

    MetaTable* Exec(MetaTable* table)

    {
        if (std::holds_alternative<std::monostate>(m_Conds.get())) {
            return table; // pas besoin dans le reflechir la comparaison est nulle
        }

        // NOT SURE
        // UUOU
        table->Selection(m_Conds, std::move(m_Cols));

        return table;
    }

    const TableNamesSet& GetTableName()
    {
        return TableNameToExec;
    }

    void NullifyCond()
    {
        m_Cols = nullptr;
        m_Conds.get().emplace<std::monostate>(std::monostate {});
    }

    const std::unordered_set<ColonneNamesSet*>& Getm_Cols()
    {
        return *m_Cols.get();
    }
};

} // Database::QueryPlanning

#endif // !SELEC_H
