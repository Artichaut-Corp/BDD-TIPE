#include "../algebrizer_types.h"
#include "../data_process_system/meta-table.h"
#include "../data_process_system/racine.h"
#include "../utils/printing_utils.h"
#include "pred.h"
#include <memory>


#ifndef JOIN_H

#define JOIN_H

namespace Database::QueryPlanning {

class Join {
private:
    Comparateur m_Comps; // la liste de vérification que deux clef des tables doivent vérifier
    std::shared_ptr<ColonneNamesSet> m_ColumnName1; // la colonne qui doit être tester par la MetaTable 1
    std::shared_ptr<ColonneNamesSet> m_ColumnName2; // la colonne qui doit être tester par la MetaTable 2

    std::shared_ptr<TableNamesSet> LTable;
    std::shared_ptr<TableNamesSet> RTable;

public:
    Join(Comparateur comps, std::shared_ptr<ColonneNamesSet> ColumnNames1, std::shared_ptr<ColonneNamesSet> ColumnNames2)
        : m_Comps(comps)
        , m_ColumnName1(ColumnNames1)
        , m_ColumnName2(ColumnNames2)
    {
        LTable = ColumnNames1->GetTableSet();
        RTable = ColumnNames2->GetTableSet();
    };

    std::shared_ptr<MetaTable> ExecNaif(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2);
    std::shared_ptr<MetaTable> ExecTrier(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2);
    std::shared_ptr<MetaTable> ExecGrouByStyle(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2); // do the same as above but presort each MetaTable

    void SetRootInfo(std::shared_ptr<TableNamesSet> LeftTableName, std::shared_ptr<TableNamesSet> RightTableName)
    {
        LTable = LeftTableName;
        RTable = RightTableName;
    }
    std::shared_ptr<TableNamesSet> GetLTable() { return LTable; }
    std::shared_ptr<TableNamesSet> GetRTable() { return RTable; }

    std::shared_ptr<ColonneNamesSet> GetLCol() { return m_ColumnName1; }
    std::shared_ptr<ColonneNamesSet> GetRCol() { return m_ColumnName2; }

    Comparateur GetComp() { return m_Comps; }
};

} // Database::QueryPlanning

#endif // !JOIN_H
