
#include "../data_process_system/table.h"
#include "../operation/agreg.h"
#include "hashmap.h"
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <vector>

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
inline void AfficheAgreg(
    std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues,
    std::vector<int>* OrdreIndice,
    std::vector<QueryPlanning::ReturnType>* OpArray)
{
    if (!ColumnNameToValues || ColumnNameToValues->empty()) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    if (OpArray->empty()) {
        std::cout << "(aucune colonne)" << std::endl;
        return;
    }

    // --- Construire la liste des colonnes et des noms à afficher ---
    std::vector<std::string> ColumnNameList;
    std::vector<std::string> DisplayNames;
    ColumnNameList.reserve(OpArray->size());
    DisplayNames.reserve(OpArray->size());

    for (auto& ret : *OpArray) {
        std::string colName = ret.GetColonne();
        ColumnNameList.push_back(colName);

        std::string display = colName;
        using AF = Parsing::AggrFuncType;
        switch (ret.GetType()) {
        case AF::AVG_F:
            display = "AVG(" + colName + ")";
            break;
        case AF::COUNT_F:
            display = "COUNT(" + colName + ")";
            break;
        case AF::MAX_F:
            display = "MAX(" + colName + ")";
            break;
        case AF::MIN_F:
            display = "MIN(" + colName + ")";
            break;
        case AF::SUM_F:
            display = "SUM(" + colName + ")";
            break;
        case AF::NOTHING_F:
        default:
            break;
        }
        DisplayNames.push_back(display);
    }

    // --- Nombre de lignes ---
    size_t nRows = 0;
    if (auto it = ColumnNameToValues->find(ColumnNameList[0]); it != ColumnNameToValues->end())
        nRows = it->second ? it->second->size() : 0;

    if (nRows == 0) {
        std::cout << "(aucune donnée)" << std::endl;
        return;
    }

    // --- Calcul des largeurs de colonnes ---
    std::vector<int> colWidths(ColumnNameList.size());
    for (size_t i = 0; i < ColumnNameList.size(); ++i) {
        const std::string& colName = ColumnNameList[i];
        colWidths[i] = (int)display_width(DisplayNames[i]);

        auto it = ColumnNameToValues->find(colName);
        if (it != ColumnNameToValues->end() && it->second) {
            for (const auto& valData : *it->second) {
                std::string val;
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                },
                    valData);
                colWidths[i] = std::max(colWidths[i], (int)display_width(val));
            }
        }
    }

    auto print_sep = [&]() {
        for (int w : colWidths)
            std::cout << "+" << std::string(w + 2, '-');
        std::cout << "+\n";
    };

    // --- Affiche l’en-tête ---
    print_sep();
    for (size_t i = 0; i < DisplayNames.size(); ++i) {
        std::cout << "| ";
        print_cell(DisplayNames[i], colWidths[i]);
        std::cout << " ";
    }
    std::cout << "|\n";
    print_sep();

    // --- Détermine l’ordre des lignes ---
    std::vector<int> ordre;
    if (OrdreIndice && !OrdreIndice->empty()) {
        for (int idx : *OrdreIndice)
            if (idx >= 0 && static_cast<size_t>(idx) < nRows)
                ordre.push_back(idx);
    } else {
        ordre.resize(nRows);
        std::iota(ordre.begin(), ordre.end(), 0);
    }

    // --- Affiche les lignes selon OrdreIndice ---
    for (int row : ordre) {
        for (size_t i = 0; i < ColumnNameList.size(); ++i) {
            const std::string& colName = ColumnNameList[i];
            std::string val;
            auto it = ColumnNameToValues->find(colName);
            if (it != ColumnNameToValues->end() && it->second && row < (int)it->second->size()) {
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                },
                    (*it->second)[row]);
            }
            std::cout << "| ";
            print_cell(val, colWidths[i]);
            std::cout << " ";
        }
        std::cout << "|\n";
    }

    print_sep();
}
inline void AfficheAgregSpan(
    std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues,
    std::span<int>* OrdreIndice,
    std::vector<QueryPlanning::ReturnType>* OpArray)
{
    if (!ColumnNameToValues || ColumnNameToValues->empty()) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    if (!OpArray || OpArray->empty()) {
        std::cout << "(aucune colonne)" << std::endl;
        return;
    }

    // --- Construire la liste des colonnes et des noms à afficher ---
    std::vector<std::string> ColumnNameList;
    std::vector<std::string> DisplayNames;
    ColumnNameList.reserve(OpArray->size());
    DisplayNames.reserve(OpArray->size());

    for (auto& ret : *OpArray) {
        std::string colName = ret.GetColonne();
        ColumnNameList.push_back(colName);

        using AF = Parsing::AggrFuncType;
        std::string display;
        switch (ret.GetType()) {
            case AF::AVG_F:   display = "AVG("   + colName + ")"; break;
            case AF::COUNT_F: display = "COUNT(" + colName + ")"; break;
            case AF::MAX_F:   display = "MAX("   + colName + ")"; break;
            case AF::MIN_F:   display = "MIN("   + colName + ")"; break;
            case AF::SUM_F:   display = "SUM("   + colName + ")"; break;
            case AF::NOTHING_F:
            default:
                display = colName;
                break;
        }
        DisplayNames.push_back(std::move(display));
    }

    // --- Nombre de lignes ---
    size_t nRows = 0;
    if (auto it = ColumnNameToValues->find(ColumnNameList[0]); it != ColumnNameToValues->end())
        nRows = it->second ? it->second->size() : 0;

    if (nRows == 0) {
        std::cout << "(aucune donnée)" << std::endl;
        return;
    }

    // --- Calcul des largeurs de colonnes ---
    std::vector<int> colWidths(ColumnNameList.size());
    for (size_t i = 0; i < ColumnNameList.size(); ++i) {
        const std::string& colName = ColumnNameList[i];
        colWidths[i] = (int)display_width(DisplayNames[i]);

        auto it = ColumnNameToValues->find(colName);
        if (it != ColumnNameToValues->end() && it->second) {
            for (const auto& valData : *it->second) {
                std::string val;
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                }, valData);
                colWidths[i] = std::max(colWidths[i], (int)display_width(val));
            }
        }
    }

    auto print_sep = [&]() {
        for (int w : colWidths)
            std::cout << "+" << std::string(w + 2, '-');
        std::cout << "+\n";
    };

    // --- Affiche l’en-tête ---
    print_sep();
    for (size_t i = 0; i < DisplayNames.size(); ++i) {
        std::cout << "| ";
        print_cell(DisplayNames[i], colWidths[i]);
        std::cout << " ";
    }
    std::cout << "|\n";
    print_sep();

    // --- Détermine l’ordre des lignes ---
    std::vector<int> ordre;
    if (!OrdreIndice->empty()) {
        ordre.reserve(OrdreIndice->size());
        for (int idx : *OrdreIndice)
            if (idx >= 0 && static_cast<size_t>(idx) < nRows)
                ordre.push_back(idx);
    } else {
        ordre.resize(nRows);
        std::iota(ordre.begin(), ordre.end(), 0);
    }

    // --- Affiche les lignes selon OrdreIndice ---
    for (int row : ordre) {
        for (size_t i = 0; i < ColumnNameList.size(); ++i) {
            const std::string& colName = ColumnNameList[i];
            std::string val;
            auto it = ColumnNameToValues->find(colName);
            if (it != ColumnNameToValues->end() && it->second && row < (int)it->second->size()) {
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                }, (*it->second)[row]);
            }
            std::cout << "| ";
            print_cell(val, colWidths[i]);
            std::cout << " ";
        }
        std::cout << "|\n";
    }

    print_sep();
}

}

#endif //! TABLE_UTILS_H
