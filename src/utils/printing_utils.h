
#include "../data_process_system/meta-table.h"
#include "../data_process_system/namingsystem.h"
#include "../operation/agreg.h"
#include "hashmap.h"
#include <iostream>
#include <memory>
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
inline void AfficheResultat(std::shared_ptr<QueryPlanning::MetaTable> table, std::vector<std::shared_ptr<QueryPlanning::ReturnType>>* OrdreRetour)
{
    if (!table) {
        std::cout << "(table vide)" << std::endl;
        return;
    }

    const size_t nb_cols = OrdreRetour->size();
    if (nb_cols == 0) {
        std::cout << "(aucune colonne à afficher)" << std::endl;
        return;
    }

    // --- Prépare les colonnes dans l'ordre de OrdreRetour ---
    std::vector< std::shared_ptr<QueryPlanning::ColonneNamesSet>> noms;
    noms.reserve(nb_cols);
    std::vector<std::shared_ptr<Database::QueryPlanning::Racine>> colonnes_a_afficher;
    colonnes_a_afficher.reserve(nb_cols);

    for (const auto ret : *OrdreRetour) {
        std::shared_ptr<QueryPlanning::ColonneNamesSet> colName = ret->GetColonne();
        noms.push_back(colName); // le nom affiché vient de OrdreRetour
        colonnes_a_afficher.push_back(table->GetTableByColName(colName)->getRacinePtr(colName));
    }

    // --- Détermine le nombre de lignes à afficher ---
    int nb_lignes = table->Columnsize();


    // --- Calcule la largeur d'affichage pour chaque colonne ---
    std::vector<int> largeurs(nb_cols);
    for (size_t i = 0; i < nb_cols; ++i) {
        int max_width = static_cast<int>(display_width(noms[i]->GetMainName()));
        const auto col = colonnes_a_afficher[i];

        if (col) {
            for (int j = 0; j < nb_lignes; j++) {
                std::string val;
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                },
                    table->get_value(col->get_name(),j));
                max_width = std::max<int>(max_width, display_width(val));
            }
        } else {
            max_width = std::max<int>(max_width, display_width("NULL"));
        }

        largeurs[i] = max_width;
    }

    // --- Affichage ---
    const auto print_sep = [&]() {
        for (size_t i = 0; i < nb_cols; ++i)
            std::cout << "+" << std::string(largeurs[i] + 2, '-');
        std::cout << "+\n";
    };

    // En-tête
    print_sep();
    for (size_t i = 0; i < nb_cols; ++i) {
        std::cout << "| ";
        print_cell(noms[i]->GetMainName(), largeurs[i]);
        std::cout << " ";
    }
    std::cout << "|\n";
    print_sep();

    // Lignes
    for (int j = 0; j < nb_lignes; ++j) {
        for (size_t i = 0; i < nb_cols; ++i) {
            const auto col = colonnes_a_afficher[i];
            std::string val;

            if (col) {
                const ColumnData& cd = table->get_value(col->get_name(),j);
                std::visit([&val](auto&& elem) {
                    using T = std::decay_t<decltype(elem)>;
                    if constexpr (std::is_same_v<T, DbString>)
                        val = Convert::DbStringToString(elem);
                    else
                        val = std::to_string(elem);
                },
                    cd);
            } else {
                val = "NULL";
            }

            std::cout << "| ";
            print_cell(val, largeurs[i]);
            std::cout << " ";
        }
        std::cout << "|\n";
    }

    print_sep();
}

// --- Affichage pour map clé → set de valeurs (après agrégation) ---
inline void AfficheAgreg(
    std::unordered_map<std::string, std::vector<ColumnData>*>* ColumnNameToValues,
    std::vector<int>* OrdreIndice,
    std::vector<std::shared_ptr<QueryPlanning::ReturnType>>* OpArray)
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
    std::vector<std::shared_ptr<QueryPlanning::ColonneNamesSet>> ColumnNameList;
    std::vector<std::string> DisplayNames;
    ColumnNameList.reserve(OpArray->size());
    DisplayNames.reserve(OpArray->size());

    for (auto& ret : *OpArray) {
        auto colName = ret->GetColonne();
        ColumnNameList.push_back(colName);

        std::string display = colName->GetMainName();
        using AF = Parsing::AggrFuncType;
        switch (ret->GetType()) {
        case AF::AVG_F:
            display = "AVG(" + display + ")";
            break;
        case AF::COUNT_F:
            display = "COUNT(" + display + ")";
            break;
        case AF::MAX_F:
            display = "MAX(" + display + ")";
            break;
        case AF::MIN_F:
            display = "MIN(" + display + ")";
            break;
        case AF::SUM_F:
            display = "SUM(" + display + ")";
            break;
        case AF::NOTHING_F:
        default:
            break;
        }
        DisplayNames.push_back(display);
    }

    // --- Nombre de lignes ---
    size_t nRows = 0;
    if (auto it = ColumnNameToValues->find(ColumnNameList[0]->GetMainName()); it != ColumnNameToValues->end())
        nRows = it->second ? it->second->size() : 0;

    if (nRows == 0) {
        std::cout << "(aucune donnée)" << std::endl;
        return;
    }

    // --- Calcul des largeurs de colonnes ---
    std::vector<int> colWidths(ColumnNameList.size());
    for (size_t i = 0; i < ColumnNameList.size(); ++i) {
        const std::shared_ptr<QueryPlanning::ColonneNamesSet> colName = ColumnNameList[i];
        colWidths[i] = (int)display_width(DisplayNames[i]);

        auto it = ColumnNameToValues->find(colName->GetMainName());
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
            const std::shared_ptr<QueryPlanning::ColonneNamesSet> colName = ColumnNameList[i];
            std::string val;
            auto it = ColumnNameToValues->find(colName->GetMainName());
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
    std::vector<std::shared_ptr<QueryPlanning::ReturnType>>* OpArray)
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
    std::vector<std::shared_ptr<QueryPlanning::ColonneNamesSet>> ColumnNameList;
    std::vector<std::string> DisplayNames;
    ColumnNameList.reserve(OpArray->size());
    DisplayNames.reserve(OpArray->size());

    for (auto& ret : *OpArray) {
        std::shared_ptr<QueryPlanning::ColonneNamesSet> colName = ret->GetColonne();
        ColumnNameList.push_back(colName);

        using AF = Parsing::AggrFuncType;
        std::string display = colName->GetMainName();
        switch (ret->GetType()) {
        case AF::AVG_F:
            display = "AVG(" + display + ")";
            break;
        case AF::COUNT_F:
            display = "COUNT(" + display + ")";
            break;
        case AF::MAX_F:
            display = "MAX(" + display + ")";
            break;
        case AF::MIN_F:
            display = "MIN(" + display + ")";
            break;
        case AF::SUM_F:
            display = "SUM(" + display + ")";
            break;
        case AF::NOTHING_F:
        default:
            display = display;
            break;
        }
        DisplayNames.push_back(std::move(display));
    }

    // --- Nombre de lignes ---
    size_t nRows = 0;
    if (auto it = ColumnNameToValues->find(ColumnNameList[0]->GetMainName()); it != ColumnNameToValues->end())
        nRows = it->second ? it->second->size() : 0;

    if (nRows == 0) {
        std::cout << "(aucune donnée)" << std::endl;
        return;
    }

    // --- Calcul des largeurs de colonnes ---
    std::vector<int> colWidths(ColumnNameList.size());
    for (size_t i = 0; i < ColumnNameList.size(); ++i) {
        const std::shared_ptr<QueryPlanning::ColonneNamesSet> colName = ColumnNameList[i];
        colWidths[i] = (int)display_width(DisplayNames[i]);

        auto it = ColumnNameToValues->find(colName->GetMainName());
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
            const std::shared_ptr<QueryPlanning::ColonneNamesSet> colName = ColumnNameList[i];
            std::string val;
            auto it = ColumnNameToValues->find(colName->GetMainName());
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

}

#endif //! TABLE_UTILS_H
