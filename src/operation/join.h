#include "../algebrizer_types.h"
#include "../data_process_system/colonne.h"
#include "../data_process_system/racine.h"
#include "../data_process_system/table.h"
#include "pred.h"
#include "../utils/table_utils.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifndef JOIN_H

#define JOIN_H

namespace Database::QueryPlanning {

class Join {
private:
    Comparateur m_Comps; // la liste de vérification que deux clef des tables doivent vérifier
    std::string m_ColumnName1; // la colonne qui doit être tester par la table 1
    std::string m_columnName2; // la colonne qui doit être tester par la table 2

    std::string LTable;
    std::string RTable;

public:
    Join(Comparateur comps, std::string ColumnNames1, std::string ColumnNames2)
        : m_Comps(comps)
        , m_ColumnName1(ColumnNames1)
        , m_columnName2(ColumnNames2)
    {
        LTable = ColumnNames1.substr(0, ColumnNames1.find("."));
        RTable = ColumnNames2.substr(0, ColumnNames2.find("."));
    };

    Table* Exec(Table* table1, Table* table2)
    {
        std::vector<std::pair<int, int>> couple_valides; // stocke tout les couple de ligne valide
        for (int i = 0; i < table1->Columnsize(); i++) {
            for (int j = 0; j < table2->Columnsize(); j++) {
                if (m_Comps.Eval(table1->get_value(m_ColumnName1, i), table2->get_value(m_columnName2, j))) {
                    couple_valides.push_back({ i, j });
                }
            }
        }

        // Vecteurs pour stocker les infos des colonnes et indices valides
        int nb_colonnes = table1->get_data_ptr()->size() + table2->get_data_ptr()->size() ;
        auto pos_valid_in_colonne = new std::vector<std::vector<int>*>(nb_colonnes); // dans ce vecteur, l'élement en position i correspondras à la liste des position toujours valide qui seras ajouter dans la colonne en poisiton i de nouvelles_colonne
        for (int k = 0; k < nb_colonnes; k++) {
            (*pos_valid_in_colonne)[k] = new std::vector<int>();
        }
        std::vector<std::pair<std::string, std::shared_ptr<Racine>>> colonnes_info; // fait la même chose qu'au dessus mais avec les autres info essentiels des colonnes

        // --- Étape 1 : Récupérer les colonnes de table1 ---
        int i = 0;
        for (std::shared_ptr<Colonne> c : *table1->get_data_ptr()) {
            std::string nom = c->get_name();
            std::shared_ptr<Racine> racine_ptr = std::make_shared<Racine>(*c->get_racine_ptr());
            colonnes_info.push_back({ nom, racine_ptr });
            i++;
        }

        // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
        for (std::shared_ptr<Colonne> c : *table2->get_data_ptr()) {
            std::string nom = c->get_name();
            std::shared_ptr<Racine> racine_ptr = std::make_shared<Racine>(*c->get_racine_ptr());
            colonnes_info.push_back({ nom, racine_ptr });
            i++;
        }

        // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---

        for (const auto& p : couple_valides) {
            int i = 0;
            for (std::shared_ptr<Colonne> c : *table1->get_data_ptr()) {
                int pos = c->get_pos_at_pos(p.first);
                (*pos_valid_in_colonne).at(i)->push_back(pos);
                i++;
            }
            for (std::shared_ptr<Colonne> c : *table2->get_data_ptr()) {
                int pos = c->get_pos_at_pos(p.second);
                (*pos_valid_in_colonne).at(i)->push_back(pos);
                i++;
            }
        }

        // --- Étape 4 : Créer les Colonnes et stocker dans shared_ptr ---
        std::vector<std::shared_ptr<Colonne>> nouvelles_colonne;

        nouvelles_colonne.reserve(colonnes_info.size());

        for (int i = 0; i < colonnes_info.size(); i++) {
            nouvelles_colonne.emplace_back(std::make_shared<Colonne>(
                colonnes_info[i].second, // std::unique_ptr<Racine>
                colonnes_info[i].first, // std::string
                *pos_valid_in_colonne->at(i) // std::vector<int>
                ));
        };
        return new Table(std::make_shared<std::vector<std::shared_ptr<Colonne>>>(nouvelles_colonne), table1->Get_name());
    }
    void SetRootInfo(std::string LeftTableName, std::string RightTableName)
    {
        LTable = LeftTableName;
        RTable = RightTableName;
    }
    std::string GetLTable() { return LTable; }
    std::string GetRTable() { return RTable; }

    std::string GetLCol() { return m_ColumnName1; }
    std::string GetRCol() { return m_columnName2; }

    Comparateur GetComp(){return m_Comps;}
};

} // Database::QueryPlanning

#endif // !JOIN_H
