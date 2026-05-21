#include "algebrizer_types.h"
#include "data_process_system/meta-table.h"
#include "data_process_system/racine.h"
#include "data_process_system/table.h"
#include "utils/printing_utils.h"

#include "pred.h"

#ifndef JOIN_H

#define JOIN_H

namespace Database::QueryPlanning {

class Join {
private:
    Comparateur m_Comps; // la liste de vérification que deux clef des tables doivent vérifier

    const ColonneNamesSet& m_ColumnName1; // la colonne qui doit être tester par la MetaTable 1
    const ColonneNamesSet& m_ColumnName2; // la colonne qui doit être tester par la MetaTable 2

    TableNamesSet& LTable;
    TableNamesSet& RTable;

public:
    Join(Comparateur comps, const ColonneNamesSet& ColumnNames1, const ColonneNamesSet& ColumnNames2)
        : m_Comps(comps)
        , m_ColumnName1(ColumnNames1)
        , m_ColumnName2(ColumnNames2)
        , LTable(*ColumnNames1.GetTableSet())
        , RTable(*ColumnNames2.GetTableSet())
    {
    }

    MetaTable* ExecNaif(MetaTable* table1, MetaTable* table2);

    MetaTable* ExecTrier(MetaTable* table1, MetaTable* table2);

    MetaTable* ExecGrouByStyle(MetaTable* table1, MetaTable* table2); // do the same as above but presort each MetaTable

    void SetRootInfo(const TableNamesSet& LeftTableName, const TableNamesSet& RightTableName) const

    {
        LTable = LeftTableName;
        RTable = RightTableName;
    }

    TableNamesSet& GetLTable() { return LTable; }
    TableNamesSet& GetRTable() { return RTable; }

    const ColonneNamesSet& GetLCol() { return m_ColumnName1; }
    const ColonneNamesSet& GetRCol() { return m_ColumnName2; }

    Comparateur GetComp() { return m_Comps; }

    float calculeRC(MetaTable* MetaTableL, MetaTable* MetaTableR, int type_of_join, int size_sample);

    int CardExecNaif(MetaTable* table1, MetaTable* table2,int size_sample_1, int size_sample_2);

    int CardExecTrier(MetaTable* table1, MetaTable* table2,int size_sample_1, int size_sample_2);

    int CardExecGrouByStyle(MetaTable* table1, MetaTable* table2,int size_sample_1, int size_sample_2);
};

} // Database::QueryPlanning

#endif // !JOIN_H
