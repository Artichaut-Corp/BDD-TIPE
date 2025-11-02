#include "../algebrizer_types.h"
#include "../data_process_system/colonne.h"
#include "../data_process_system/racine.h"
#include "../data_process_system/table.h"
#include "../utils/printing_utils.h"
#include "pred.h"

#ifndef JOIN_H

#define JOIN_H

namespace Database::QueryPlanning {

class Join {
private:
    Comparateur m_Comps; // la liste de vérification que deux clef des tables doivent vérifier
    ColonneNamesSet* m_ColumnName1; // la colonne qui doit être tester par la table 1
    ColonneNamesSet* m_ColumnName2; // la colonne qui doit être tester par la table 2

    TableNamesSet* LTable;
    TableNamesSet* RTable;

public:
    Join(Comparateur comps, ColonneNamesSet* ColumnNames1, ColonneNamesSet* ColumnNames2)
        : m_Comps(comps)
        , m_ColumnName1(ColumnNames1)
        , m_ColumnName2(ColumnNames2)
    {
        LTable = ColumnNames1->GetTableSet();
        RTable = ColumnNames2->GetTableSet();
    };

    Table* ExecNaif(Table* table1, Table* table2);
    Table* ExecTrier(Table* table1, Table* table2);

    void SetRootInfo(TableNamesSet* LeftTableName, TableNamesSet* RightTableName)
    {
        LTable = LeftTableName;
        RTable = RightTableName;
    }
    TableNamesSet* GetLTable() { return LTable; }
    TableNamesSet* GetRTable() { return RTable; }

    ColonneNamesSet* GetLCol() { return m_ColumnName1; }
    ColonneNamesSet* GetRCol() { return m_ColumnName2; }

    Comparateur GetComp() { return m_Comps; }
};

} // Database::QueryPlanning

#endif // !JOIN_H
