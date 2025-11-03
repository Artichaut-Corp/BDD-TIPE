#include "join.h"
#include "../algebrizer_types.h"
#include "../data_process_system/colonne.h"
#include "../data_process_system/racine.h"
#include "../data_process_system/table.h"
#include "../utils/printing_utils.h"
#include "pred.h"
#include <memory>
#include <utility>
#include <vector>

namespace Database::QueryPlanning {
Table* Join::ExecNaif(Table* table1, Table* table2)
{
    std::vector<std::pair<int, int>> couple_valides; // stocke tout les couple de ligne valide
    for (int i = 0; i < table1->Columnsize(); i++) {
        auto val1 = table1->get_value(m_ColumnName1, i);
        for (int j = 0; j < table2->Columnsize(); j++) {
            auto val2 = table2->get_value(m_ColumnName2, j);
            if (m_Comps.Eval(val1, val2)) {
                couple_valides.push_back({ i, j });
            }
        }
    }
    // Vecteurs pour stocker les infos des colonnes et indices valides
    int nb_colonnes = table1->GetColumnNames()->size() + table2->GetColumnNames()->size() - 1;
    std::vector<std::vector<int>> pos_valid_in_colonne(nb_colonnes);
    for (int k = 0; k < nb_colonnes; k++) {
        (pos_valid_in_colonne)[k] = std::vector<int>();
    }
    std::vector<std::pair<ColonneNamesSet*, std::shared_ptr<Racine>>> colonnes_info; // fait la même chose qu'au dessus mais avec les autres info essentiels des colonnes
    colonnes_info.reserve(nb_colonnes);
    // --- Étape 1 : Récupérer les colonnes de table1 ---
    for (ColonneNamesSet* name : *table1->GetColumnNames()) {
        auto c = table1->GetMap()[name->GetMainName()];
        if (*name == *m_ColumnName1) {
            name->FusionColumn((table2->GetMap()[m_ColumnName2->GetMainName()]->get_name())); // fusionne leur nom connu
        }
        std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
        colonnes_info.push_back({ name, racine_ptr });
    }

    // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
    for (ColonneNamesSet* name : *table2->GetColumnNames()) {
        if (*name != *m_ColumnName2) {
            ColonneNamesSet* nom = name;
            auto c = table2->GetMap()[name->GetMainName()];
            std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
            colonnes_info.push_back({ nom, racine_ptr });
        }
    }

    // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---

    for (const auto& p : couple_valides) {
        int i = 0;
        for (ColonneNamesSet* name : *table1->GetColumnNames()) {
            auto c = table1->GetMap()[name->GetMainName()];
            int pos = c->get_pos_at_pos(p.first);
            pos_valid_in_colonne.at(i).push_back(pos);
            i++;
        }
        for (ColonneNamesSet* name : *table2->GetColumnNames()) {
            if (*name != *m_ColumnName2) {
                auto c = table2->GetMap()[name->GetMainName()];
                int pos = c->get_pos_at_pos(p.second);
                pos_valid_in_colonne.at(i).push_back(pos);
                i++;
            }
        }
    }

    // --- Étape 4 : Créer les Colonnes et stocker dans shared_ptr ---
    std::vector<std::shared_ptr<Colonne>> nouvelles_colonne;

    nouvelles_colonne.reserve(colonnes_info.size());

    for (int i = 0; i < colonnes_info.size(); i++) {
        auto heap_vec = new std::vector<int>(std::move(pos_valid_in_colonne[i]));
        nouvelles_colonne.emplace_back(std::make_shared<Colonne>(
            colonnes_info[i].second, // std::shared_ptr<Racine>
            colonnes_info[i].first, // ColonneNamesSet *
            heap_vec // std::vector<int>
            ));
    };

    return new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(nouvelles_colonne), table1->Get_name());
}

Table* Join::ExecTrier(Table* table1, Table* table2) // do the same as above but presort each table
{
    std::vector<std::pair<int, int>> couple_valides; // stocke tout les couple de ligne valide

    // --- Étape 0 : Trier chacune des table en fonction de la colonne---
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
                    couple_valides.push_back({ i, j });
                }
            }
        }
    }
    
    // Vecteurs pour stocker les infos des colonnes et indices valides
    int nb_colonnes = table1->GetColumnNames()->size() + table2->GetColumnNames()->size() - 1;
    std::vector<std::vector<int>> pos_valid_in_colonne(nb_colonnes);
    for (int k = 0; k < nb_colonnes; k++) {
        (pos_valid_in_colonne)[k] = std::vector<int>();
    }
    std::vector<std::pair<ColonneNamesSet*, std::shared_ptr<Racine>>> colonnes_info; // fait la même chose qu'au dessus mais avec les autres info essentiels des colonnes
    colonnes_info.reserve(nb_colonnes);
    // --- Étape 1 : Récupérer les colonnes de table1 ---
    for (ColonneNamesSet* name : *table1->GetColumnNames()) {
        auto c = table1->GetMap()[name->GetMainName()];
        if (*name == *m_ColumnName1) {
            name->FusionColumn((table2->GetMap()[m_ColumnName2->GetMainName()]->get_name())); // fusionne leur nom connu
        }
        std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
        colonnes_info.push_back({ name, racine_ptr });
    }

    // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
    for (ColonneNamesSet* name : *table2->GetColumnNames()) {
        if (*name != *m_ColumnName2) {
            ColonneNamesSet* nom = name;
            auto c = table2->GetMap()[name->GetMainName()];
            std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
            colonnes_info.push_back({ nom, racine_ptr });
        }
    }

    // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---

    for (const auto& p : couple_valides) {
        int i = 0;
        for (ColonneNamesSet* name : *table1->GetColumnNames()) {
            auto c = table1->GetMap()[name->GetMainName()];
            int pos = c->get_pos_at_pos(p.first);
            pos_valid_in_colonne.at(i).push_back(pos);
            i++;
        }
        for (ColonneNamesSet* name : *table2->GetColumnNames()) {
            if (*name != *m_ColumnName2) {
                auto c = table2->GetMap()[name->GetMainName()];
                int pos = c->get_pos_at_pos(p.second);
                pos_valid_in_colonne.at(i).push_back(pos);
                i++;
            }
        }
    }

    // --- Étape 4 : Créer les Colonnes et stocker dans shared_ptr ---
    std::vector<std::shared_ptr<Colonne>> nouvelles_colonne;

    nouvelles_colonne.reserve(colonnes_info.size());

    for (int i = 0; i < colonnes_info.size(); i++) {
        auto heap_vec = new std::vector<int>(std::move(pos_valid_in_colonne[i]));
        nouvelles_colonne.emplace_back(std::make_shared<Colonne>(
            colonnes_info[i].second, // std::shared_ptr<Racine>
            colonnes_info[i].first, // ColonneNamesSet *
            heap_vec // std::vector<int>
            ));
    };

    return new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(nouvelles_colonne), table1->Get_name());
}

Table* Join::ExecGrouByStyle(Table* table1, Table* table2) // do the same as above but presort each table
{
    std::vector<std::pair<int, int>> couple_valides; // stocke tout les couple de ligne valide
    
    std::unordered_map<ColumnData,std::pair<std::vector<int>,std::vector<int>>> map_col;
    for (int i =0; i<table1->Columnsize();i++){
        map_col[table1->get_value(m_ColumnName1,i)].first.push_back(i);
    }
    for (int i = 0; i<table2->Columnsize();i++){
        map_col[table2->get_value(m_ColumnName2,i)].second.push_back(i);
    }
    for(auto it = map_col.begin(); it != map_col.end(); ++it) {
        auto values = it->second;
        for (auto i : values.first){
            for (auto j : values.second){
                couple_valides.push_back({i,j});
            }
        }
    }
    // Vecteurs pour stocker les infos des colonnes et indices valides
    int nb_colonnes = table1->GetColumnNames()->size() + table2->GetColumnNames()->size() - 1;
    std::vector<std::vector<int>> pos_valid_in_colonne(nb_colonnes);
    for (int k = 0; k < nb_colonnes; k++) {
        (pos_valid_in_colonne)[k] = std::vector<int>();
    }
    std::vector<std::pair<ColonneNamesSet*, std::shared_ptr<Racine>>> colonnes_info; // fait la même chose qu'au dessus mais avec les autres info essentiels des colonnes
    colonnes_info.reserve(nb_colonnes);
    // --- Étape 1 : Récupérer les colonnes de table1 ---
    for (ColonneNamesSet* name : *table1->GetColumnNames()) {
        auto c = table1->GetMap()[name->GetMainName()];
        if (*name == *m_ColumnName1) {
            name->FusionColumn((table2->GetMap()[m_ColumnName2->GetMainName()]->get_name())); // fusionne leur nom connu
        }
        std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
        colonnes_info.push_back({ name, racine_ptr });
    }

    // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
    for (ColonneNamesSet* name : *table2->GetColumnNames()) {
        if (*name != *m_ColumnName2) {
            ColonneNamesSet* nom = name;
            auto c = table2->GetMap()[name->GetMainName()];
            std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
            colonnes_info.push_back({ nom, racine_ptr });
        }
    }

    // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---

    for (const auto& p : couple_valides) {
        int i = 0;
        for (ColonneNamesSet* name : *table1->GetColumnNames()) {
            auto c = table1->GetMap()[name->GetMainName()];
            int pos = c->get_pos_at_pos(p.first);
            pos_valid_in_colonne.at(i).push_back(pos);
            i++;
        }
        for (ColonneNamesSet* name : *table2->GetColumnNames()) {
            if (*name != *m_ColumnName2) {
                auto c = table2->GetMap()[name->GetMainName()];
                int pos = c->get_pos_at_pos(p.second);
                pos_valid_in_colonne.at(i).push_back(pos);
                i++;
            }
        }
    }

    // --- Étape 4 : Créer les Colonnes et stocker dans shared_ptr ---
    std::vector<std::shared_ptr<Colonne>> nouvelles_colonne;

    nouvelles_colonne.reserve(colonnes_info.size());

    for (int i = 0; i < colonnes_info.size(); i++) {
        auto heap_vec = new std::vector<int>(std::move(pos_valid_in_colonne[i]));
        nouvelles_colonne.emplace_back(std::make_shared<Colonne>(
            colonnes_info[i].second, // std::shared_ptr<Racine>
            colonnes_info[i].first, // ColonneNamesSet *
            heap_vec // std::vector<int>
            ));
    };

    return new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(nouvelles_colonne), table1->Get_name());
}
};