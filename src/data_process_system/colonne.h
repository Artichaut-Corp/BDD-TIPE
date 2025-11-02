#include "../operation/pred.h"
#include "racine.h"

#include <iostream>
#include <memory>
#include <vector>

#ifndef COLONNE_OP_H

#define COLONNE_OP_H

namespace Database::QueryPlanning {

// Colonne : contient un pointeur vers une racine + indices valides + nom de la colonne
class Colonne {
private:
    std::shared_ptr<Racine> m_Racine;
    std::vector<int> m_Indices; // indices valides dans racine
    ColonneNamesSet* m_NomColonne; // nom de la colonne de la forme "Table.nom_colonne"

public:
    Colonne(std::shared_ptr<Racine> racine_, ColonneNamesSet* nom_colonne_)
        : m_Racine(racine_)
        , m_NomColonne(nom_colonne_)
    {
        std::vector<int> temp;
        temp.reserve(m_Racine->size());
        for (int i = 0; i < m_Racine->size(); i++) {
            temp.push_back(i);
        }
        m_Indices = temp;
    }

    Colonne(std::shared_ptr<Racine> racine_,  ColonneNamesSet* nom_, const std::vector<int>* indices_)
        : m_Racine(racine_)
        , m_NomColonne(nom_)
        , m_Indices(*indices_)
    {
    }

    // Accès à une valeur via l'indice de la colonne
    ColumnData getValue(const int idx) const
    {
        return m_Racine->getValue(m_Indices[idx]);
    }

    int size() const
    {
        return m_Indices.size();
    }

    void
    print() const
    {
        for (int i = 0; i < m_Indices.size(); i++) {
            Database::QueryPlanning::afficherColumnData(m_Racine->getValue((m_Indices)[i]));
        }
        std::cout << "\n";
    }

    ColonneNamesSet* get_name()
    {
        return m_NomColonne;
    }
    std::shared_ptr<Racine> get_racine_ptr()
    {
        return m_Racine;
    }
    int get_pos_at_pos(int i)
    {
        return m_Indices[i];
    }
    void garder_indice_valide(std::shared_ptr<std::vector<int>> indices_valide)
    {
        std::vector<int> filtré;
        filtré.reserve(indices_valide->size()); 

        for (int idx : *indices_valide) {
            if (idx < m_Indices.size()) {
                filtré.push_back(m_Indices[idx]);
            }
        }
        m_Indices = filtré;
    }

    void AppliqueOrdre(std::vector<int>* OrdreAApplique){
        std::vector<int> NouveauOrdreIndice;
        NouveauOrdreIndice.reserve(m_Indices.size());
        for (auto e : *OrdreAApplique){
            NouveauOrdreIndice.push_back(m_Indices[e]);
        }
        m_Indices = NouveauOrdreIndice;
    }
};

}

#endif // !COLONNE_OP_H
