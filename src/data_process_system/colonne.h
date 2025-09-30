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
    std::shared_ptr<Racine> racine;
    std::vector<int> indices; // indices valides dans racine
    std::string nom_colonne; // nom de la colonne de la forme "Table.nom_colonne"

public:
    Colonne(std::shared_ptr<Racine> racine_, const std::string& nom_colonne_)
        : racine(racine_)
        , nom_colonne(nom_colonne_)
    {
        std::vector<int> temp;
        temp.reserve(racine->size());
        for (int i = 0; i < racine->size(); i++) {
            temp.push_back(i);
        }
        indices = temp;
    }

    Colonne(std::shared_ptr<Racine> racine_, const std::string& nom_, const std::vector<int>& indices_)
        : racine(std::move(racine_))
        , nom_colonne(nom_)
        , indices(indices_)
    {
    }

    // Accès à une valeur via l'indice de la colonne
    ColumnData getValue(const int idx) const
    {
        return racine->getValue((indices)[idx]);
    }

    int size() const
    {
        return indices.size();
    }

    void
    print() const
    {
        for (int i = 0; i < indices.size(); i++) {
            Database::QueryPlanning::afficherColumnData(racine->getValue((indices)[i]));
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
    int get_pos_at_pos(int i)
    {
        return indices[i];
    }
    void garder_indice_valide(std::shared_ptr<std::vector<int>> indices_valide)
    {
        std::vector<int> filtré;
        filtré.reserve(indices_valide->size()); // optimisation (merci chat gpt)

        for (int idx : *indices_valide) {
            if (idx < indices.size()) {
                filtré.push_back(indices[idx]);
            }
        }
        indices = filtré;
    }
};

}

#endif // !COLONNE_OP_H
