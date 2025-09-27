#include "agreg.h"
#include <iomanip>
#include <iostream>

namespace Database::QueryPlanning {


void Final::AfficheResultat(Table* table) {
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
    for (auto const& [nom, pos] : table->GetMap()) {
        noms.resize(std::max(noms.size(), pos + 1));
        noms[pos] = nom;
    }

    // Détermine le nombre de lignes (taille max des colonnes)
    size_t nb_lignes = 0;
    for (auto const& col : *colonnes) {
        nb_lignes = std::max(nb_lignes, col->size());
    }

    // Calcule la largeur max pour chaque colonne
    std::vector<size_t> largeurs(noms.size(), 0);
    for (size_t i = 0; i < noms.size(); ++i) {
        largeurs[i] = noms[i].size();
        for (size_t j = 0; j < nb_lignes; ++j) {
            std::string val;
            ColumnData cd = (*colonnes)[i]->getValue(j);
            std::visit([&val](auto&& elem) {
                using T = std::decay_t<decltype(elem)>;
                if constexpr (std::is_same_v<T, DbString>) {
                    val = Convert::DbStringToString(elem);
                } else if constexpr (std::is_integral_v<T>) {
                    val = std::to_string(elem);
                }
            }, cd);
            largeurs[i] = std::max(largeurs[i], val.size());
        }
    }

    auto print_sep = [&]() {
        for (size_t i = 0; i < noms.size(); ++i) {
            std::cout << "+" << std::string(largeurs[i] + 2, '-');
        }
        std::cout << "+" << std::endl;
    };

    // Affichage entête
    print_sep();
    for (size_t i = 0; i < noms.size(); ++i) {
        std::cout << "| " << std::setw(largeurs[i]) << std::left << noms[i] << " ";
    }
    std::cout << "|" << std::endl;
    print_sep();

    // Affichage lignes
    for (size_t j = 0; j < nb_lignes; ++j) {
        for (size_t i = 0; i < noms.size(); ++i) {
            std::string val;
            ColumnData cd = (*colonnes)[i]->getValue(j);
            std::visit([&val](auto&& elem) {
                using T = std::decay_t<decltype(elem)>;
                if constexpr (std::is_same_v<T, DbString>) {
                    val = Convert::DbStringToString(elem);
                } else if constexpr (std::is_integral_v<T>) {
                    val = std::to_string(elem);
                }
            }, cd);
            std::cout << "| " << std::setw(largeurs[i]) << std::left << val << " ";
        }
        std::cout << "|" << std::endl;
    }

    print_sep();
}
}