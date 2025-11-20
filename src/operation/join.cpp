#include "join.h"
#include "../algebrizer_types.h"
#include "../data_process_system/meta-table.h"
#include "../data_process_system/racine.h"
#include "../utils/printing_utils.h"
#include "pred.h"
#include <utility>
#include <vector>

namespace Database::QueryPlanning {
std::shared_ptr<MetaTable> Join::ExecNaif(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2)
{
    std::pair<std::vector<int>, std::vector<int>> couple_valides; // stocke tout les couple de ligne valide
    for (int i = 0; i < table1->Columnsize(); i++) {
        auto val1 = table1->get_value(m_ColumnName1, i);
        for (int j = 0; j < table2->Columnsize(); j++) {
            auto val2 = table2->get_value(m_ColumnName2, j);
            if (m_Comps.Eval(val1, val2)) {
                couple_valides.first.push_back(i);
                couple_valides.second.push_back(j);
            }
        }
    }

    table1->AppliqueOrdre(&(couple_valides.first));
    table2->AppliqueOrdre(&(couple_valides.second));
    auto racineCol1 = table1->GetTableByColName(m_ColumnName1)->getRacinePtr(m_ColumnName1);
    auto racineCol2 = table2->GetTableByColName(m_ColumnName2)->getRacinePtr(m_ColumnName2);
    racineCol1->addname(racineCol2->get_name());
    table2->GetTableByColName(m_ColumnName2)->DeleteCol(m_ColumnName2);

    table1->FusionMetaTable(table2);

    return table1;
}

std::shared_ptr<MetaTable> Join::ExecTrier(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2) // do the same as above but presort each MetaTable
{
    std::pair<std::vector<int>, std::vector<int>> couple_valides; // stocke tout les couple de ligne valide

    // --- Étape 0 : Trier chacune des MetaTable en fonction de la colonne---
    auto taille = table2->size();
    table1->Sort(m_ColumnName1);
    table2->Sort(m_ColumnName2);
    int pos1 = 0;
    int pos2 = 0;
    while (pos1 < table1->Columnsize() && pos2 < table2->Columnsize()) {
        auto val1 = table1->get_value(m_ColumnName1, pos1);
        auto val2 = table2->get_value(m_ColumnName2, pos2);

        if (val1 < val2) {
            ++pos1;
        } else if (val1 > val2) {
            ++pos2;
        } else {
            auto mainval = val1;
            int pos1deb = pos1;
            int pos2deb = pos2;

            while (pos1 < table1->Columnsize() && table1->get_value(m_ColumnName1, pos1) == mainval)
                ++pos1;
            while (pos2 < table2->Columnsize() && table2->get_value(m_ColumnName2, pos2) == mainval)
                ++pos2;

            for (int i = pos1deb; i < pos1; ++i) {
                for (int j = pos2deb; j < pos2; ++j) {
                    couple_valides.first.push_back(i);
                    couple_valides.second.push_back(j);
                }
            }
        }
    }
    table1->AppliqueOrdre(&(couple_valides.first));
    table2->AppliqueOrdre(&(couple_valides.second));
    auto racineCol1 = table1->GetTableByColName(m_ColumnName1)->getRacinePtr(m_ColumnName1);
    auto racineCol2 = table2->GetTableByColName(m_ColumnName2)->getRacinePtr(m_ColumnName2);
    racineCol1->addname(racineCol2->get_name());
    table2->GetTableByColName(m_ColumnName2)->DeleteCol(m_ColumnName2);

    table1->FusionMetaTable(table2);

    return table1;
}

std::shared_ptr<MetaTable> Join::ExecGrouByStyle(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2) // do the same as above but presort each MetaTable
{
    std::pair<std::vector<int>, std::vector<int>> couple_valides; // stocke tout les couple de ligne valide

    std::unordered_map<ColumnData, std::vector<int>> map_col;
    for (int i = 0; i < table1->Columnsize(); i++) {
        map_col[table1->get_value(m_ColumnName1, i)].push_back(i);
    }
    for (int i = 0; i < table2->Columnsize(); i++) {
        for (auto j : map_col[table2->get_value(m_ColumnName2, i)]) {
            couple_valides.first.push_back(i);
            couple_valides.second.push_back(j);
        }
    }
    table1->AppliqueOrdre(&(couple_valides.first));
    table2->AppliqueOrdre(&(couple_valides.second));
    auto racineCol1 = table1->GetTableByColName(m_ColumnName1)->getRacinePtr(m_ColumnName1);
    auto racineCol2 = table2->GetTableByColName(m_ColumnName2)->getRacinePtr(m_ColumnName2);
    racineCol1->addname(racineCol2->get_name());
    table2->GetTableByColName(m_ColumnName2)->DeleteCol(m_ColumnName2);

    table1->FusionMetaTable(table2);

    return table1;
}
};