#include "../Operation/pred.h"
#include "Racine.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
// Colonne : contient un pointeur vers une racine + indices valides + nom de la colonne
class Colonne {
private:
    std::shared_ptr<Racine> racine;
    std::vector<size_t> indices; // indices valides dans racine
    std::string nom_colonne;
public:
    Colonne(std::shared_ptr<Racine> racine_,  std::string nom_colonne_,  std::vector<size_t> indices_ = {})
        : racine(racine_)
        , indices(indices_)
        , nom_colonne(nom_colonne_)
    {
        if (indices.empty()) {
            indices.reserve(racine->size());
            for (size_t i = 0; i < racine->size(); i++) {
                indices.push_back(i);
            }
        }
    }

    // Accès à une valeur via l'indice de la colonne
    Database::Querying::ColumnData getValue(const size_t idx) const
    {
        return racine->getValue(indices[idx]);
    }

    size_t size() const
    {
        return indices.size();
    }

    // Projection : filtrer les indices selon un prédicat
    std::vector<size_t> Projection(const Database::Querying::Predicat_list& pred)
    {
        std::vector<size_t> indices_supprimes;
        for (int i; i < indices.size(); i++) {
            bool a_supprimer = !pred.Eval(racine->getValue(i));
            if (a_supprimer) {
                indices_supprimes.push_back(i);
            }
        }
        return indices_supprimes;
    }

    void Enleve_indices(const std::vector<size_t>& list_indices)
    {
        for (size_t idx : list_indices) {
            if (idx < indices.size()) {
                indices.erase(indices.begin() + idx);
            }
        }
    }

    void
    print() const
    {
        for (size_t i = 0; i < indices.size(); i++) {
            Database::Querying::afficherColumnData(racine->getValue(indices[i]));
        }
        std::cout << "\n";
    }

    size_t get_pos_at_pos(int i)
    {
        return indices[i];
    }
    std::string get_name()
    {
        return nom_colonne;
    }
    std::shared_ptr<Racine> get_racine_ptr()
    {
        return racine;
    }
};
