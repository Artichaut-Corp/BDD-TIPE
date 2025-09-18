#include "../operation/pred.h"
#include "racine.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifndef COLONNE_OP_H

#define COLONNE_OP_H

namespace Database::QueryPlanning {

// Colonne : contient un pointeur vers une racine + indices valides + nom de la colonne
class Colonne {
private:
    std::unique_ptr<Racine> racine;
    std::unique_ptr<std::vector<size_t>> indices; // indices valides dans racine
    std::string nom_colonne; // nom de la colonne de la forme "Table.nom_colonne"

public:
    Colonne(Racine* racine_, const std::string& nom_colonne_, std::vector<size_t>* indices_ = {})
        : racine(racine_)
        , indices(indices_)
        , nom_colonne(nom_colonne_)
    {
        if (indices->empty()) {
            indices->reserve(racine->size());
            for (size_t i = 0; i < racine->size(); i++) {
                indices->push_back(i);
            }
        }
    }

    // Accès à une valeur via l'indice de la colonne
    ColumnData getValue(const size_t idx) const
    {
        return racine->getValue((*indices)[idx]);
    }

    size_t size() const
    {
        return indices->size();
    }

    void
    print() const
    {
        for (size_t i = 0; i < indices->size(); i++) {
            Database::Querying::afficherColumnData(racine->getValue((*indices)[i]));
        }
        std::cout << "\n";
    }

    std::string get_name()
    {
        return nom_colonne;
    }
    Racine* get_racine_ptr()
    {
        return racine.get();
    }
    std::vector<size_t>* get_indices()
    {
        return indices.get();
    }
    size_t get_pos_at_pos(int i)
    {
        return (*indices)[i];
    }
    void garder_indice_valide(std::unique_ptr<std::vector<size_t>> indices_valide)
    {
        std::vector<size_t> filtré;
        filtré.reserve(indices_valide->size()); // optimisation (merci chat gpt)

        for (size_t idx : *indices_valide) {
            if (idx < indices->size()) {
                filtré.push_back((*indices)[idx]);
            }
        }
        indices = std::make_unique<std::vector<size_t>>(filtré);
    }
};

}

#endif // !COLONNE_OP_H
