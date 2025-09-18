#include "../algebrizer_types.h"
#include "../data_process_system/colonne.h"
#include "../data_process_system/racine.h"
#include "../data_process_system/table.h"
#include "pred.h"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#ifndef JOIN_H

#define JOIN_H

namespace Database::QueryPlanning {

class Join {
private:
    Comparateur m_Comps; // la liste de vérification que deux clef des tables doivent vérifier
    std::string m_ColumnName1; // la colonne qui doit être tester par la table 1
    std::string m_columnName2; //la colonne qui doit être tester par la table 2

    struct pair_hash {
        std::size_t operator()(const std::pair<size_t, size_t>& p) const
        {
            return std::hash<size_t>()(p.first) ^ (std::hash<size_t>()(p.second) << 1);
        }
    };

public:
    Join(Comparateur comps, std::string ColumnNames1, std::string ColumnNames2)
        : m_Comps(comps)
        , m_ColumnName1(ColumnNames1)
        , m_columnName2(ColumnNames2) { };

    std::unique_ptr<Table> Exec(std::unique_ptr<Table> table1, std::unique_ptr<Table> table2)
    {
        // Pour faire une jointure on doit procéder par deux étape
        // 1) repérer tout les couples valides
        // 2)recréer une nouvelle Table en rejoignant les deux Tables avec les lignes que l'on garde
        // 1) :
        std::unordered_set<std::pair<size_t, size_t>, pair_hash> couple_valides; // stocke tout les couple de ligne valide
        for (int i = 0; i < table1->size(); i++) {
            for (int j = 0; j < table2->size(); j++) {
                if (m_Comps.Eval(table1->get_value(m_ColumnName1, i), table2->get_value(m_columnName2, j))) {
                    couple_valides.insert({ i, j });
                }
            }
        }

        // Vecteurs pour stocker les infos des colonnes et indices valides
        std::vector<std::unique_ptr<Colonne>> nouvelles_colonne;
        std::vector<std::vector<size_t>> pos_valid_in_colonne; // dans ce vecteur, l'élement en position i correspondras à la liste des position toujours valide qui seras ajouter dans la colonne en poisiton i de nouvelles_colonne
        std::vector<std::pair<std::string, std::unique_ptr<Racine>>> colonnes_info; // fait la même chose qu'au dessus mais avec les autres info essentiels des colonnes
        std::map<std::string, size_t> map;

        // --- Étape 1 : Récupérer les colonnes de table1 ---
        int i = 0;
        for (std::unique_ptr<Colonne> c : *table1->get_data_ptr()) {
            std::unique_ptr<std::string> nom = c->get_name();
            std::unique_ptr<Racine> racine_ptr = c->get_racine_ptr();
            map[*nom] = i;
            colonnes_info.push_back({ *nom, racine_ptr });
            pos_valid_in_colonne.emplace_back(); // préparer un vecteur pour les indices valides
            i++;
        }

        // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
        for (std::unique_ptr<Colonne> c : *table2->get_data_ptr()) {
            if (!table1->colonne_exist(c->get_name())) {
                std::string nom = *c->get_name();
                std::unique_ptr<Racine> racine_ptr = c->get_racine_ptr();
                map[nom] = i;
                colonnes_info.push_back({ nom, racine_ptr });
                pos_valid_in_colonne.emplace_back();
                i++;
            }
        }

        // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---
        for (const auto& p : couple_valides) {
            int i = 0;
            for (std::unique_ptr<Colonne> c : *table1->get_data_ptr()) {
                pos_valid_in_colonne[i].push_back(c->get_pos_at_pos(p.first));
                i++;
            }
            for (std::unique_ptr<Colonne> c : *table2->get_data_ptr()) {
                if (!table1->colonne_exist(c->get_name())) {
                    pos_valid_in_colonne[i].push_back(c->get_pos_at_pos(p.second));
                    i++;
                }
            }
        }

        // --- Étape 4 : Créer les Colonnes et stocker dans unique_ptr ---
        for (size_t i = 0; i < colonnes_info.size(); i++) {
            nouvelles_colonne.push_back(std::make_shared<Colonne>(
                colonnes_info[i].second, // std::unique_ptr<Racine>
                colonnes_info[i].first, // std::string
                pos_valid_in_colonne[i] // std::vector<size_t>
                ));
        };
        std::vector<std::string> names;

        names.reserve(map.size());

        for (const auto& i : map) {
            names.emplace_back(i.first);
        }

        return std::make_shared<Table>(Table((std::make_shared<std::vector<std::unique_ptr<Colonne>>>(nouvelles_colonne)), names));
    }
};

} // Database::QueryPlanning

#endif // !JOIN_H
