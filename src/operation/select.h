#include "algebrizer_types.h"
#include "data_process_system/meta-table.h"
#include "parser/expression.h"

#include "pred.h"

#include <cassert>
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
    std::unique_ptr<Parsing::BinaryExpression::Condition> m_Conds;

    // set of all the column who are being checked
    std::unique_ptr<std::unordered_set<ColonneNamesSet*>> m_Cols;

public:
    Select(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> cols, std::unique_ptr<Parsing::BinaryExpression::Condition> cond, const TableNamesSet& Table)
        : TableNameToExec(Table)
        , m_Cols(std::move(cols))
        , m_Conds(std::move(cond)) {};

    Parsing::BinaryExpression::Condition* GetCond() const { return m_Conds.get(); }

    MetaTable* Exec(MetaTable* table)
    {
        if (std::holds_alternative<std::monostate>(*m_Conds)) {
            return table; // pas besoin dans le reflechir la comparaison est nulle
        }

        // NOT SURE
        // UUOU
        table->Selection(*m_Conds, std::move(m_Cols));

        return table;
    }

    const TableNamesSet& GetTableName()
    {
        return TableNameToExec;
    }

    const std::unordered_set<ColonneNamesSet*>& Getm_Cols()
    {
        return *m_Cols.get();
    }

    std::unique_ptr<Parsing::BinaryExpression::Condition> ExtractCond()
    {
        std::unique_ptr<Parsing::BinaryExpression::Condition> tmp = std::move(m_Conds); 

        m_Conds = std::make_unique<Parsing::BinaryExpression::Condition>(std::monostate {});

        return tmp;
    }
};

} // Database::QueryPlanning

#endif // !SELEC_H
