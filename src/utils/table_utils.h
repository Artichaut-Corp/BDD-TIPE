
#include <iomanip>
#include <iostream>
#include "../data_process_system/table.h"


#ifndef TABLE_UTILS_H
#define TABLE_UTILS_H

namespace Database::Utils {

inline void AfficheResultat(QueryPlanning::Table* table)
{
    if (!table) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    auto colonnes = table->get_data_ptr();
    if (!colonnes || colonnes->empty()) {
        std::cout << "(table sans colonnes)" << std::endl;
        return;
    }

    // Récupère les noms des colonnes
    std::vector<std::string> noms;
    for (auto const& [nom, _] : table->GetMap()) {
        //noms.resize(pos + 1 > noms.size() ? pos + 1 : noms.size());
        noms.push_back(nom);
    }

    // Détermine le nombre de lignes (taille max des colonnes)
    int nb_lignes = 0;
    for (auto const& col : *colonnes) {
        nb_lignes = std::max(nb_lignes, col->size());
    }

    // Calcule la largeur max pour chaque colonne
    std::vector<int> largeurs(noms.size(), 0);
    for (int i = 0; i < noms.size(); ++i) {
        largeurs[i] = noms[i].size();
        for (int j = 0; j < nb_lignes; ++j) {
            std::string val;
            ColumnData cd = (*colonnes)[i]->getValue(j);
            std::visit([&val](auto&& elem) {
                using T = std::decay_t<decltype(elem)>;
                if constexpr (std::is_same_v<T, DbString>) {
                    val = Convert::DbStringToString(elem);
                } else if constexpr (std::is_integral_v<T>) {
                    val = std::to_string(elem);
                }
            },
                cd);
            largeurs[i] = largeurs[i] > val.size() ? largeurs[i] : val.size();
        }
    }

    auto print_sep = [&]() {
        for (int i = 0; i < noms.size(); ++i) {
            std::cout << "+" << std::string(largeurs[i] + 2, '-');
        }
        std::cout << "+" << std::endl;
    };

    // Affichage entête
    print_sep();
    for (int i = 0; i < noms.size(); ++i) {
        std::cout << "| " << std::setw(largeurs[i]) << std::left << noms[i] << " ";
    }
    std::cout << "|" << std::endl;
    print_sep();

    // Affichage lignes
    for (int j = 0; j < nb_lignes; ++j) {
        for (int i = 0; i < noms.size(); ++i) {
            std::string val;
            ColumnData cd = (*colonnes)[i]->getValue(j);
            std::visit([&val](auto&& elem) {
                using T = std::decay_t<decltype(elem)>;
                if constexpr (std::is_same_v<T, DbString>) {
                    val = Convert::DbStringToString(elem);
                } else if constexpr (std::is_integral_v<T>) {
                    val = std::to_string(elem);
                }
            },
                cd);
            std::cout << "| " << std::setw(largeurs[i]) << std::left << val << " ";
        }
        std::cout << "|" << std::endl;
    }

    print_sep();
}
}
#endif //!TABLE_UTILS_H
