#include "join.h"
#include "algebrizer_types.h"
#include "data_process_system/meta-table.h"
#include "data_process_system/racine.h"
#include "pred.h"
#include "utils/printing_utils.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace Database::QueryPlanning {

MetaTable* Join::ExecNaif(MetaTable* meta_table1, MetaTable* meta_table2)
{
    // stocke tout les couple de ligne valide
    auto couple_valides = std::make_pair(std::make_unique<std::vector<int>>(), std::make_unique<std::vector<int>>());

    auto val1 = meta_table1->GetValue(m_ColumnName1, 0);
    auto val2 = meta_table2->GetValue(m_ColumnName2, 0);

    int MT1size = meta_table1->Columnsize();
    int MT2size = meta_table2->Columnsize();
    for (int i = 0; i < MT1size; i++) {

        val1 = meta_table1->GetValue(m_ColumnName1, i);

        for (int j = 0; j < MT2size; j++) {

            val2 = meta_table2->GetValue(m_ColumnName2, j);

            if (m_Comps.Eval(val1, val2)) {
                couple_valides.first->push_back(i);
                couple_valides.second->push_back(j);
            }
        }
    }

    meta_table1->AppliqueOrdre(*couple_valides.first.get());

    meta_table2->AppliqueOrdre(*couple_valides.second.get());

    auto racineCol1 = meta_table1->GetTableByColName(m_ColumnName1).GetRacinePtr(m_ColumnName1);

    auto racineCol2 = meta_table2->GetTableByColName(m_ColumnName2).GetRacinePtr(m_ColumnName2);

    racineCol1->AddName(racineCol2->GetName());

    meta_table2->GetTableByColName(m_ColumnName2).DeleteCol(m_ColumnName2);

    meta_table1->FusionMetaTable(*meta_table2);

    return meta_table1;
}

// do the same as above but presort each MetaTable
MetaTable* Join::ExecTrier(MetaTable* meta_table1, MetaTable* meta_table2)
{
    // stocke tout les couple de ligne valide
    auto couple_valides = std::make_pair(std::make_unique<std::vector<int>>(), std::make_unique<std::vector<int>>());

    // --- Étape 0 : Trier chacune des MetaTable en fonction de la colonne---
    meta_table1->Sort(m_ColumnName1);

    meta_table2->Sort(m_ColumnName2);

    int pos1 = 0;

    int pos2 = 0;
    auto val1 = meta_table1->GetValue(m_ColumnName1, pos1);
    auto val2 = meta_table2->GetValue(m_ColumnName2, pos2);

    int MT1size = meta_table1->Columnsize();
    int MT2size = meta_table2->Columnsize();
    while (pos1 < MT1size && pos2 < MT2size) {

        auto val1 = meta_table1->GetValue(m_ColumnName1, pos1);

        if (val1 < val2) {
            pos1++;
            val1 = meta_table1->GetValue(m_ColumnName1, pos1);

        } else if (val1 > val2) {
            pos2++;
            val2 = meta_table2->GetValue(m_ColumnName2, pos2);

        } else {
            auto mainval = val1;
            int pos1deb = pos1;
            int pos2deb = pos2;
            while (val1 == mainval) {
                if (pos1 + 1 >= MT1size) {
                    break;
                }
                pos1++;
                val1 = meta_table1->GetValue(m_ColumnName1, pos1);
            }
            while (val2 == mainval) {
                if (pos2 + 1 >= MT2size) {
                    break;
                }
                pos2++;
                val2 = meta_table2->GetValue(m_ColumnName2, pos2);
            }

            for (int i = pos1deb; i < pos1; i++) {
                for (int j = pos2deb; j < pos2; j++) {
                    couple_valides.first->push_back(i);
                    couple_valides.second->push_back(j);
                }
            }
        }
    }

    meta_table1->AppliqueOrdre(*couple_valides.first.get());

    meta_table2->AppliqueOrdre(*couple_valides.second.get());

    auto racineCol1 = meta_table1->GetTableByColName(m_ColumnName1).GetRacinePtr(m_ColumnName1);

    auto racineCol2 = meta_table2->GetTableByColName(m_ColumnName2).GetRacinePtr(m_ColumnName2);

    racineCol1->AddName(racineCol2->GetName());

    meta_table2->GetTableByColName(m_ColumnName2).DeleteCol(m_ColumnName2);

    meta_table1->FusionMetaTable(*meta_table2);

    return meta_table1;
}

// do the same as above but presort each MetaTable
MetaTable* Join::ExecGrouByStyle(MetaTable* meta_table1, MetaTable* meta_table2)
{
    // stocke tout les couple de ligne valide
    auto couple_valides = std::make_pair(std::make_unique<std::vector<int>>(), std::make_unique<std::vector<int>>());

    std::unordered_map<ColumnData, std::vector<int>> map_col;

    int MT1size = meta_table1->Columnsize();
    int MT2size = meta_table2->Columnsize();

    for (int i = 0; i < MT1size; i++) {
        map_col[meta_table1->GetValue(m_ColumnName1, i)].push_back(i);
    }

    for (int i = 0; i < MT2size; i++) {
        auto result = map_col[meta_table2->GetValue(m_ColumnName2, i)];
        for (auto j : result) {

            couple_valides.first->push_back(j);
            couple_valides.second->push_back(i);
        }
    }

    meta_table1->AppliqueOrdre(*couple_valides.first.get());

    meta_table2->AppliqueOrdre(*couple_valides.second.get());

    auto racineCol1 = meta_table1->GetTableByColName(m_ColumnName1).GetRacinePtr(m_ColumnName1);

    auto racineCol2 = meta_table2->GetTableByColName(m_ColumnName2).GetRacinePtr(m_ColumnName2);

    racineCol1->AddName(racineCol2->GetName());

    meta_table2->GetTableByColName(m_ColumnName2).DeleteCol(m_ColumnName2);

    meta_table1->FusionMetaTable(*meta_table2);

    return meta_table1;
}

int Join::CardExecNaif(MetaTable* meta_table1, MetaTable* meta_table2, int size_sample_1, int size_sample_2)
{
    auto Sample1 = meta_table1->GetSampleFromColumn(m_ColumnName1, size_sample_1);

    auto Sample2 = meta_table2->GetSampleFromColumn(m_ColumnName2, size_sample_2);

    int nbr_match = 0;
    auto val1 = (*Sample1)[0];
    auto val2 = (*Sample2)[0];

    for (int i = 0; i < Sample1->size(); i++) {

        val1 = (*Sample1)[i];

        for (int j = 0; j < Sample2->size(); j++) {

            val2 = (*Sample2)[j];

            if (m_Comps.Eval(val1, val2)) {
                nbr_match++;
            }
        }
    }

    return nbr_match;
}

int Join::CardExecTrier(MetaTable* meta_table1, MetaTable* meta_table2, int size_sample_1, int size_sample_2)
{
    auto Sample1 = meta_table1->GetSampleFromColumn(m_ColumnName1, size_sample_1);

    auto Sample2 = meta_table2->GetSampleFromColumn(m_ColumnName2, size_sample_2);

    sort(Sample1->begin(), Sample1->end());

    sort(Sample2->begin(), Sample2->end());

    int nbr_match = 0;

    int pos1 = 0;
    int pos2 = 0;
    auto val1 = (*Sample1)[0];
    auto val2 = (*Sample2)[0];
    int sizesample1 = Sample1->size();
    int sizesample2 = Sample2->size();

    while (pos1 < Sample1->size() && pos2 < Sample2->size()) {
        val1 = (*Sample1)[pos1];
        val2 = (*Sample2)[pos2];

        if (val1 < val2) {
            ++pos1;
        } else if (val1 > val2) {
            ++pos2;
        } else {
            auto mainval = val1;
            int pos1deb = pos1;
            int pos2deb = pos2;

            while (pos1 < sizesample1 && (*Sample1)[pos1] == mainval)
                pos1++;
            while (pos2 < sizesample2 && (*Sample2)[pos2] == mainval)
                pos2++;
            nbr_match += (pos1 - pos1deb) * (pos2 - pos2deb);
        }
    }

    return nbr_match;
}

int Join::CardExecGrouByStyle(MetaTable* meta_table1, MetaTable* meta_table2, int size_sample_1, int size_sample_2)
{
    auto Sample1 = meta_table1->GetSampleFromColumn(m_ColumnName1, size_sample_1);

    auto Sample2 = meta_table2->GetSampleFromColumn(m_ColumnName2, size_sample_2);

    int nbr_match = 0;

    std::unordered_map<ColumnData, int> map_col;

    for (int i = 0; i < Sample1->size(); i++) {
        map_col[(*Sample1)[i]]++;
    }

    for (int i = 0; i < Sample2->size(); i++) {
        nbr_match += map_col[(*Sample2)[i]];
    }

    return nbr_match;
}

float Join::calculeRC(MetaTable* MetaTableL, MetaTable* MetaTableR, int type_of_join, int size_sample)
{
    float CardResult;

    if (type_of_join == 0) {
        CardResult = CardExecNaif(MetaTableL, MetaTableR, size_sample, size_sample);
    } else if (type_of_join == 1) {
        CardResult = CardExecTrier(MetaTableL, MetaTableR, size_sample, size_sample);
    } // else if (JoinParam == 2    ) {
    //    CardResult = CardExecTrierStockerMemoire(MetaTableL, MetaTableR);
    //}
    else if (type_of_join == 3) {
        CardResult = CardExecGrouByStyle(MetaTableL, MetaTableR, size_sample, size_sample);
    } else {
        throw std::runtime_error("Type de Join Inconnu");
    }
    int max_meta = MetaTableL->Columnsize() * MetaTableR->Columnsize();

    return (CardResult / max_meta);
}

};
