
#include "../data_process_system/table.h"
#include <iostream>

#ifndef TABLE_UTILS_H
#define TABLE_UTILS_H

namespace Database::Utils {

// Compte le nombre de caractères visibles (UTF-8 aware)
inline size_t display_width(const std::string& s)
{
    size_t count = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) { // on ne compte que les octets de début
            ++count;
        }
    }
    return count;
}

// Imprime une cellule avec padding correct (UTF-8 aware)
inline void print_cell(const std::string& s, int width)
{
    size_t visible = display_width(s);
    int padding = width - static_cast<int>(visible);
    std::cout << std::left << s << std::string(padding, ' ');
}

// Fonction d’affichage du résultat
inline void AfficheResultat(QueryPlanning::Table table)
{
    if (table.isUninitialized()) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    std::unique_ptr<std::vector<QueryPlanning::Colonne>> colonnes = table.getTableCols();

    if (!colonnes || colonnes->empty()) {
        std::cout << "(table sans colonnes)" << std::endl;
        return;
    }

    // Récupère les noms des colonnes
    std::vector<std::string> noms;

    auto map = table.GetMap();

    for (auto const& [nom, _] : *map) {
        noms.push_back(nom);
    }

    // Détermine le nombre de lignes
    int nb_lignes = 0;
    for (auto const& col : *colonnes) {
        nb_lignes = std::max(nb_lignes, (int)col.size());
    }

    // Calcule la largeur max pour chaque colonne
    std::vector<int> largeurs(noms.size(), 0);

    for (int i = 0; i < (int)noms.size(); ++i) {
        largeurs[i] = (int)display_width(noms[i]);
        for (int j = 0; j < nb_lignes; ++j) {
            std::string val;
            ColumnData cd = (*colonnes)[i].getValue(j);
            std::visit([&val](auto&& elem) {
                using T = std::decay_t<decltype(elem)>;
                if constexpr (std::is_same_v<T, DbString>) {
                    val = Convert::DbStringToString(elem);
                } else {
                    val = std::to_string(elem);
                }
            },
                cd);
            largeurs[i] = std::max<int>(largeurs[i], (int)display_width(val));
        }
    }

    auto print_sep = [&]() {
        for (int i = 0; i < (int)noms.size(); ++i) {
            std::cout << "+" << std::string(largeurs[i] + 2, '-');
        }
        std::cout << "+" << std::endl;
    };

    // Affichage entête
    print_sep();
    for (int i = 0; i < (int)noms.size(); ++i) {
        std::cout << "| ";
        print_cell(noms[i], largeurs[i]);
        std::cout << " ";
    }
    std::cout << "|" << std::endl;
    print_sep();

    // Affichage lignes
    for (int j = 0; j < nb_lignes; ++j) {
        for (int i = 0; i < (int)noms.size(); ++i) {

            std::string val;

            ColumnData cd = (*colonnes)[i].getValue(j);

            std::visit([&val](auto&& elem) {
                using T = std::decay_t<decltype(elem)>;
                if constexpr (std::is_same_v<T, DbString>) {
                    val = Convert::DbStringToString(elem);
                } else {
                    val = std::to_string(elem);
                }
            },
                cd);

            std::cout << "| ";
            print_cell(val, largeurs[i]);
            std::cout << " ";
        }
        std::cout << "|" << std::endl;
    }

    print_sep();
}
}
#endif //! TABLE_UTILS_H
