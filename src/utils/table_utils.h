
#include "../data_process_system/table.h"
#include "hashmap.h"
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
        noms.push_back(nom);
    }

    // Détermine le nombre de lignes
    int nb_lignes = 0;
    for (auto const& col : *colonnes) {
        nb_lignes = std::max(nb_lignes, (int)col->size());
    }

    // Calcule la largeur max pour chaque colonne
    std::vector<int> largeurs(noms.size(), 0);
    for (int i = 0; i < (int)noms.size(); ++i) {
        largeurs[i] = (int)display_width(noms[i]);
        for (int j = 0; j < nb_lignes; ++j) {
            std::string val;
            ColumnData cd = (*colonnes)[i]->getValue(j);
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
            ColumnData cd = (*colonnes)[i]->getValue(j);
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
// --- Affichage pour map clé → set de valeurs (après agrégation) ---
inline void AfficheAgregWithGroupByResult(
    const std::vector<std::string>& groupByCols,
    const std::vector<std::string>& agregCols,
    const std::unordered_map<std::string, Database::Utils::Hash::MultiValueMapDyn>& AgregMap)
{
    if (groupByCols.empty() && agregCols.empty()) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    // Construire l'entête : groupBy + agreg
    std::vector<std::string> allCols = groupByCols;
    allCols.insert(allCols.end(), agregCols.begin(), agregCols.end());

    // Calculer la largeur max pour chaque colonne
    std::vector<int> colWidths(allCols.size(), 0);
    for (size_t i = 0; i < allCols.size(); ++i)
        colWidths[i] = (int)display_width(allCols[i]);

    // On suppose que toutes les map ont les mêmes clés
    if (!agregCols.empty()) {
        auto& firstMap = AgregMap.at(agregCols[0]);
        for (const auto& [key, values] : firstMap) {
            for (size_t j = 0; j < key.keys.size(); ++j) {
                std::string val;
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                },
                    key.keys[j]);
                colWidths[j] = std::max(colWidths[j], (int)display_width(val));
            }

            size_t colOffset = key.keys.size();
            for (size_t k = 0; k < agregCols.size(); ++k) {
                if (!values.empty()) {
                    std::string val;
                    std::visit([&val](auto&& elem) {
                        using T = std::decay_t<decltype(elem)>;
                        if constexpr (std::is_same_v<T, DbString>)
                            val = Convert::DbStringToString(elem);
                        else
                            val = std::to_string(elem);
                    },
                        *values.begin());
                    colWidths[colOffset + k] = std::max(colWidths[colOffset + k], (int)display_width(val));
                }
            }
        }
    }

    auto print_sep = [&]() {
        for (int w : colWidths)
            std::cout << "+" << std::string(w + 2, '-');
        std::cout << "+" << std::endl;
    };

    // Affichage entête
    print_sep();
    for (size_t i = 0; i < allCols.size(); ++i) {
        std::cout << "| ";
        print_cell(allCols[i], colWidths[i]);
        std::cout << " ";
    }
    std::cout << "|" << std::endl;
    print_sep();

    // Affichage lignes
    if (!agregCols.empty()) {
        auto& firstMap = AgregMap.at(agregCols[0]);
        for (const auto& [key, values] : firstMap) {
            for (size_t j = 0; j < key.keys.size(); ++j) {
                std::string val;
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                },
                    key.keys[j]);
                std::cout << "| ";
                print_cell(val, colWidths[j]);
                std::cout << " ";
            }

            // Affichage valeurs agrégées
            size_t colOffset = key.keys.size();
            for (size_t k = 0; k < agregCols.size(); ++k) {
                auto itMap = AgregMap.find(agregCols[k]);
                std::string val;
                if (itMap != AgregMap.end()) {
                    auto itKey = itMap->second.find(key);
                    if (itKey != itMap->second.end() && !itKey->second.empty()) {
                        std::visit([&val](auto&& elem) {
                            using T = std::decay_t<decltype(elem)>;
                            if constexpr (std::is_same_v<T, DbString>)
                                val = Convert::DbStringToString(elem);
                            else
                                val = std::to_string(elem);
                        },
                            *itKey->second.begin());
                    }
                }
                std::cout << "| ";
                print_cell(val, colWidths[colOffset + k]);
                std::cout << " ";
            }
            std::cout << "|" << std::endl;
        }
    }

    print_sep();
}
inline void AfficheAgregNoGroupByResult(
    const std::vector<std::string>& colNames,
    const std::vector<Database::ColumnData>& row)
{
    if (colNames.empty() || row.empty()) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    // Calculer largeur max pour chaque colonne
    std::vector<int> colWidths(colNames.size(), 0);
    for (size_t i = 0; i < colNames.size(); ++i)
        colWidths[i] = (int)Database::Utils::display_width(colNames[i]);

    for (size_t i = 0; i < row.size(); ++i) {
        std::string val;
        std::visit([&val](auto&& elem) {
            using T = std::decay_t<decltype(elem)>;
            if constexpr (std::is_same_v<T, Database::DbString>)
                val = Convert::DbStringToString(elem);
            else
                val = std::to_string(elem);
        },
            row[i]);
        colWidths[i] = std::max(colWidths[i], (int)Database::Utils::display_width(val));
    }

    auto print_sep = [&]() {
        for (int w : colWidths)
            std::cout << "+" << std::string(w + 2, '-');
        std::cout << "+" << std::endl;
    };

    // Affichage entête
    print_sep();
    for (size_t i = 0; i < colNames.size(); ++i) {
        std::cout << "| ";
        Database::Utils::print_cell(colNames[i], colWidths[i]);
        std::cout << " ";
    }
    std::cout << "|" << std::endl;
    print_sep();

    // Affichage unique ligne
    for (size_t i = 0; i < row.size(); ++i) {
        std::string val;
        std::visit([&val](auto&& elem) {
            using T = std::decay_t<decltype(elem)>;
            if constexpr (std::is_same_v<T, Database::DbString>)
                val = Convert::DbStringToString(elem);
            else
                val = std::to_string(elem);
        },
            row[i]);
        std::cout << "| ";
        Database::Utils::print_cell(val, colWidths[i]);
        std::cout << " ";
    }
    std::cout << "|" << std::endl;

    print_sep();
}

}
#endif //! TABLE_UTILS_H
