#include <cmath>
#include <memory>
#include <numeric>
#include <unordered_map>
#include <vector>

#include "../algebrizer_types.h"
#include "namingsystem.h"
#include "racine.h"
#ifndef TABLE_OP

#define TABLE_OP

namespace Database::QueryPlanning {

class Table {
private:
    std::unordered_map<std::string, std::shared_ptr<Racine>> m_Map; // permet de trouver la position d'une colonne à partir de son nom
    std::vector<std::shared_ptr<Racine>> m_Colonnes; // contient les noms de toute les colonnes présente dans la table (de manière unique) avec table étant la table originel( pas la table qui est crée par le progamme mais celle qui est présent en mémoire) et la colonne associé à celle-ci
    std::vector<int> m_Indices; // indices valides dans racine
    std::shared_ptr<TableNamesSet> m_Name;

public:
    Table(std::vector<std::shared_ptr<Racine>>* data_, std::shared_ptr<TableNamesSet> name_)
        : m_Name(std::move(name_))
    {
        for (auto e : *data_) {
            for (auto n : e->get_name()->GetAllFullNames()) {
                m_Map[n] = e;
            }
            m_Map[e->get_name()->GetMainName()] = e;
            m_Colonnes.push_back(e);
        }
        std::vector<int> temp;
        temp.reserve((*data_)[0]->size());
        for (int i = 0; i < (*data_)[0]->size(); i++) {
            temp.push_back(i);
        }
        m_Indices = temp;
    }

    int size()
    {
        return m_Map.size();
    }
    int Columnsize()
    {
        return m_Indices.size();
    }

    ColumnData get_value_dans_table(std::shared_ptr<ColonneNamesSet> column_name, int pos_ind)
    {
        return m_Map[column_name->GetMainName()]->get_value_dans_ptr(m_Indices[pos_ind]);
    }

    

    bool colonne_exist(std::shared_ptr<ColonneNamesSet> clef_testé)
    {
        return !(m_Map.end() == m_Map.find(clef_testé->GetMainName())); // this test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
    }
    std::shared_ptr<Racine> getRacinePtr(std::shared_ptr<ColonneNamesSet> colname)
    {
        return m_Map[colname->GetMainName()];
    };

    
    std::vector<std::shared_ptr<Racine>>* GetColumns() { return &m_Colonnes; }

    std::shared_ptr<TableNamesSet> get_name() { return m_Name; }

    std::vector<int>* Sort(std::shared_ptr<ColonneNamesSet> ColonneToSortBy)
    {
        auto ColonneSorting = m_Map[ColonneToSortBy->GetMainName()];

        std::vector<int>* PosInColonneToSortBy = new std::vector<int>(m_Indices.size());
        std::iota(PosInColonneToSortBy->begin(), PosInColonneToSortBy->end(), 0);

        std::sort(PosInColonneToSortBy->begin(), PosInColonneToSortBy->end(),
            [&](int a, int b) { return this->get_value_dans_table(ColonneToSortBy,m_Indices[a] ) < this->get_value_dans_table(ColonneToSortBy, m_Indices[b]); });
        return PosInColonneToSortBy;
    }

    void AppliqueFiltre(std::vector<int>* new_ind)
    {
        std::vector<int> new_indices;
        new_indices.reserve(new_ind->size());
        for (auto e : *new_ind) {
            new_indices.push_back(m_Indices[e]);
        }
        m_Indices = new_indices;
    }

    void DeleteCol(std::shared_ptr<ColonneNamesSet> DeletedCol)
    {
        for (int i = 0; i < m_Colonnes.size(); i++) {
            if (*m_Colonnes[i]->get_name() == *DeletedCol) {
                for (auto s : DeletedCol->GetAllFullNames()) {
                    m_Map.erase(s);
                }
                m_Colonnes.erase(m_Colonnes.begin() + i);
                break;
            }
        }
        update();
    }
    void update()
    {
        m_Map.erase(m_Map.begin(), m_Map.end());

        for (auto r : m_Colonnes) {
            for (auto n : r->get_name()->GetAllFullNames()) {
                m_Map[n] = r;
            }
        }
    }
};

} // Database::QueryPlanning
//
#endif // !TABLE_OP
