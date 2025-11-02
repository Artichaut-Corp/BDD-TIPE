#include "../algebrizer_types.h"
#include "../data_process_system/colonne.h"
#include "../data_process_system/racine.h"
#include "../data_process_system/table.h"
#include "../utils/printing_utils.h"
#include "pred.h"
#include <memory>
#include <utility>
#include <vector>

#ifndef JOIN_H

#define JOIN_H

namespace Database::QueryPlanning {

class Join {
private:
    Comparateur m_Comps; // la liste de vérification que deux clef des tables doivent vérifier
    ColonneNamesSet* m_ColumnName1; // la colonne qui doit être tester par la table 1
    ColonneNamesSet* m_columnName2; // la colonne qui doit être tester par la table 2

    TableNamesSet* LTable;
    TableNamesSet* RTable;

public:
    Join(Comparateur comps, ColonneNamesSet* ColumnNames1, ColonneNamesSet* ColumnNames2)
        : m_Comps(comps)
        , m_ColumnName1(ColumnNames1)
        , m_columnName2(ColumnNames2)
    {
        LTable = ColumnNames1->GetTableSet();
        RTable = ColumnNames2->GetTableSet();
    };

    Table* Exec(Table* table1, Table* table2)
    {

        std::vector<std::pair<int, int>> couple_valides; // stocke tout les couple de ligne valide
        for (int i = 0; i < table1->Columnsize(); i++) {
            auto val1 = table1->get_value(m_ColumnName1, i);
            for (int j = 0; j < table2->Columnsize(); j++) {
                auto val2 = table2->get_value(m_columnName2, j);
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
        for (ColonneNamesSet * name : *table1->GetColumnNames()) {
            auto c = table1->GetMap()[name->GetMainName()];
            if (*name == *m_ColumnName1) {
                name->FusionColumn((table2->GetMap()[m_columnName2->GetMainName()]->get_name())); // fusionne leur nom connu
            }
            std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
            colonnes_info.push_back({ name, racine_ptr });
        }

        // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
        for (ColonneNamesSet * name : *table2->GetColumnNames()) {
            if (*name != *m_columnName2) {
                ColonneNamesSet* nom = name;
                auto c = table2->GetMap()[name->GetMainName()];
                std::shared_ptr<Racine> racine_ptr = c->get_racine_ptr();
                colonnes_info.push_back({ nom, racine_ptr });
            }
        }

        // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---

        for (const auto& p : couple_valides) {
            int i = 0;
            for (ColonneNamesSet * name : *table1->GetColumnNames()) {
                auto c = table1->GetMap()[name->GetMainName()];
                int pos = c->get_pos_at_pos(p.first);
                pos_valid_in_colonne.at(i).push_back(pos);
                i++;
            }
            for (ColonneNamesSet * name : *table2->GetColumnNames()) {
                if (*name != *m_columnName2) {
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
    void SetRootInfo(TableNamesSet* LeftTableName, TableNamesSet* RightTableName)
    {
        LTable = LeftTableName;
        RTable = RightTableName;
    }
    TableNamesSet* GetLTable() { return LTable; }
    TableNamesSet* GetRTable() { return RTable; }

    ColonneNamesSet* GetLCol() { return m_ColumnName1; }
    ColonneNamesSet* GetRCol() { return m_columnName2; }

    Comparateur GetComp() { return m_Comps; }
};

} // Database::QueryPlanning

#endif // !JOIN_H
