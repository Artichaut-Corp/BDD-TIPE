#include "../operation/pred.h"
#include "racine.h"

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
    Racine m_Racine;

    std::unique_ptr<std::vector<int>> m_Indices; // indices valides dans racine
  
    std::string m_ColumnName; // nom de la colonne de la forme "Table.nom_colonne"

public:
    Colonne(Racine racine, const std::string& column_name)
        : m_Racine(racine)
        , m_ColumnName(column_name)
    {

        auto temp = std::make_unique<std::vector<int>>();

        temp->reserve(racine.size());

        for (int i = 0; i < racine.size(); i++) {
            temp->emplace_back(i);
        }

        m_Indices = std::move(temp);
    }

    Colonne(Racine racine, const std::string& name, const std::vector<int>& indices)
        : m_Racine(racine)
        , m_ColumnName(name)
        , m_Indices(std::make_unique<std::vector<int>>(indices))
    {
    }

    // Accès à une valeur via l'indice de la colonne
    ColumnData getValue(const int idx) const
    {
        return m_Racine.getValue(m_Indices->at(idx));
    }

    int size() const
    {
        return m_Indices->size();
    }

    void
    Print() const
    {
        for (int i = 0; i < m_Indices->size(); i++) {
            Database::QueryPlanning::afficherColumnData(m_Racine.getValue(m_Indices->at(i)));
        }
        std::cout << "\n";
    }

    std::string getName()
    {
        return m_ColumnName;
    }

    Racine getRacine()
    {
        return m_Racine;
    }

    int getPosAtPos(int i)
    {
        return m_Indices->at(i);
    }

    void garder_indice_valide(std::unique_ptr<std::vector<int>> indices_valide)
    {
        auto filtred = std::make_unique<std::vector<int>>();

        filtred->reserve(indices_valide->size()); // optimisation (merci chat gpt)

        for (int idx : *indices_valide) {

            if (idx < m_Indices->size()) {

                filtred->emplace_back(m_Indices->at(idx));
            }
        }

        m_Indices = std::move(filtred);
    }
};

}

#endif // !COLONNE_OP_H
