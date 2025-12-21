#include "join.h"
#include "../algebrizer_types.h"
#include "../data_process_system/meta-table.h"
#include "../data_process_system/racine.h"
#include "../utils/printing_utils.h"
#include "pred.h"
#include <algorithm>
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
            couple_valides.first.push_back(j);
            couple_valides.second.push_back(i);
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

int Join::CardExecNaif(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2)
{
    auto Sample1 = table1->GetSample(m_ColumnName1);
    auto Sample2 = table2->GetSample(m_ColumnName2);
    int nbr_match = 0;
    for (int i = 0; i < Sample1->size(); i++) {
        auto val1 = (*Sample1)[i];
        for (int j = 0; j < Sample2->size(); j++) {
            auto val2 = (*Sample2)[j];
            if (m_Comps.Eval(val1, val2)) {
                nbr_match++;
            }
        }
    }
    return nbr_match;
}
int Join::CardExecTrier(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2)
{
    auto Sample1 = table1->GetSample(m_ColumnName1);
    auto Sample2 = table2->GetSample(m_ColumnName2);
    sort(Sample1->begin(), Sample1->end());
    sort(Sample2->begin(), Sample2->end());
    int nbr_match = 0;

    int pos1 = 0;
    int pos2 = 0;
    while (pos1 < Sample1->size() && pos2 < Sample2->size()) {
        auto val1 = (*Sample1)[pos1];
        auto val2 = (*Sample2)[pos2];

        if (val1 < val2) {
            ++pos1;
        } else if (val1 > val2) {
            ++pos2;
        } else {
            auto mainval = val1;
            int pos1deb = pos1;
            int pos2deb = pos2;

            while (pos1 < Sample1->size() && (*Sample1)[pos1] == mainval)
                ++pos1;
            while (pos2 < Sample2->size() && (*Sample2)[pos2] == mainval)
                ++pos2;
            nbr_match += (pos1 - pos1deb) * (pos2 - pos2deb);
        }
    }
    return nbr_match;
}
int Join::CardExecGrouByStyle(std::shared_ptr<MetaTable> table1, std::shared_ptr<MetaTable> table2)
{
    auto Sample1 = table1->GetSample(m_ColumnName1);
    auto Sample2 = table2->GetSample(m_ColumnName2);
    int nbr_match = 0;

    std::unordered_map<ColumnData, std::vector<int>> map_col;
    for (int i = 0; i < Sample1->size(); i++) {
        map_col[(*Sample1)[i]].push_back(i);
    }
    for (int i = 0; i < Sample2->size(); i++) {
        nbr_match += map_col[(*Sample2)[i]].size();
    }
    return nbr_match;
}

float Join::calculeRC(std::shared_ptr<MetaTable> MetaTableL, std::shared_ptr<MetaTable> MetaTableR, int type_of_join)
{
    float CardResult;
    if (type_of_join == 0) {
        CardResult = CardExecNaif(MetaTableL, MetaTableR);
    } else if (type_of_join == 1) {
        CardResult = CardExecTrier(MetaTableL, MetaTableR);
    } // else if (JoinParam == 2    ) {
    //    CardResult = CardExecTrierStockerMemoire(MetaTableL, MetaTableR);
    //}
    else if (type_of_join == 3) {
        CardResult = CardExecGrouByStyle(MetaTableL, MetaTableR);
    } else {
        throw std::runtime_error("Type de Join Inconnu");
    }
    int max_meta = std::max(MetaTableL->Columnsize(), MetaTableR->Columnsize());
    if (max_meta > 1000) {
        return (CardResult / 1000);
    } else {
        return (CardResult / max_meta);
    }
}

};