#include "../algebrizer_types.h"
#include "../data_process_system/colonne.h"
#include "../data_process_system/racine.h"
#include "../data_process_system/table.h"
#include "pred.h"

#include <algorithm>
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
    Table m_Table;
    std::vector<Comparateur_list> m_Comps; // la liste de vérification que deux clef des tables doivent vérifier
    std::vector<std::string> m_ColumnNames; // la liste des colonnes qui doivent être tester

    struct pair_hash {
        std::size_t operator()(const std::pair<size_t, size_t>& p) const
        {
            return std::hash<size_t>()(p.first) ^ (std::hash<size_t>()(p.second) << 1);
        }
    };

public:
    Join(std::vector<Comparateur_list> comps, Table table, std::vector<std::string> ColumnNames)
        : m_Table(table)
        , m_Comps(comps)
        , m_ColumnNames(ColumnNames) { };

    Table Exec(Table table1, Table table2)
    {
        // Pour faire une jointure on doit procéder par deux étape
        // 1) repérer tout les couples valides
        // 2)recréer une nouvelle Table en rejoignant les deux Tables avec les lignes que l'on garde

        // 1) :
        std::unordered_set<std::pair<size_t, size_t>, pair_hash> couple_valides; // stocke tout les couple de ligne valide
        for (int i = 0; i < table1.size(); i++) {
            for (int j = 0; j < table2.size(); j++) {
                bool valid = true;
                for (int k = 0; k < m_Comps.size(); k++) {
                    if (!m_Comps[k].Eval(table1.get_value(m_ColumnNames[k], i), table2.get_value(m_ColumnNames[k], j))) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    couple_valides.insert({ i, j });
                }
            }
        }

        // Vecteurs pour stocker les infos des colonnes et indices valides
        std::vector<Colonne> nouvelles_colonne;
        std::vector<std::vector<size_t>> pos_valid_in_colonne; // dans ce vecteur, l'élement en position i correspondras à la liste des position toujours valide qui seras ajouter dans la colonne en poisiton i de nouvelles_colonne
        std::vector<std::pair<std::string, std::shared_ptr<Racine>>> colonnes_info; // fait la même chose qu'au dessus mais avec les autres info essentiels des colonnes
        std::map<std::string, size_t> map;

        // --- Étape 1 : Récupérer les colonnes de table1 ---
        int i = 0;
        for (Colonne c : table1.get_data_ptr()) {
            std::string nom = c.get_name();
            std::shared_ptr<Racine> racine_ptr = c.get_racine_ptr();
            map[nom] = i;
            colonnes_info.push_back({ nom, racine_ptr });
            pos_valid_in_colonne.emplace_back(); // préparer un vecteur pour les indices valides
            i++;
        }

        // --- Étape 2 : Ajouter les colonnes uniques de table2 ---
        for (Colonne c : table2.get_data_ptr()) {
            if (!table1.colonne_exist(c.get_name())) {
                std::string nom = c.get_name();
                std::shared_ptr<Racine> racine_ptr = c.get_racine_ptr();
                map[nom] = i;
                colonnes_info.push_back({ nom, racine_ptr });
                pos_valid_in_colonne.emplace_back();
                i++;
            }
        }

        // --- Étape 3 : Remplir pos_valid_in_colonne selon les couples valides ---
        for (const auto& p : couple_valides) {
            int i = 0;
            for (Colonne c : table1.get_data_ptr()) {
                pos_valid_in_colonne[i].push_back(c.get_pos_at_pos(p.first));
                i++;
            }
            for (Colonne c : table2.get_data_ptr()) {
                if (!table1.colonne_exist(c.get_name())) {
                    pos_valid_in_colonne[i].push_back(c.get_pos_at_pos(p.second));
                    i++;
                }
            }
        }

        // --- Étape 4 : Créer les Colonnes et stocker dans unique_ptr ---
        for (size_t i = 0; i < colonnes_info.size(); i++) {
            nouvelles_colonne.push_back(
                Colonne( // le constructeur n'est pas trouvé je ne sait pas
                    colonnes_info[i].second, // std::shared_ptr<Racine>
                    colonnes_info[i].first, // std::string
                    pos_valid_in_colonne[i] // std::vector<size_t>
                    ));
        }

        std::vector<std::string> names;

        names.reserve(map.size());

        for (const auto& i : map) {
            names.emplace_back(i.first);
        }

        m_Table = Table(nouvelles_colonne, names);
    }
};

} // Database::QueryPlanning

#endif // !JOIN_H
