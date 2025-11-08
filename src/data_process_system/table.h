#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../algebrizer_types.h"
#include "colonne.h"
#include "namingsystem.h"

#ifndef TABLE_OP

#define TABLE_OP

namespace Database::QueryPlanning {

class Table {
private:
    std::unordered_map<std::string, std::shared_ptr<Colonne>> m_Map; // permet de trouver la position d'une colonne à partir de son nom
    std::vector<ColonneNamesSet*> m_ColonnesNames; // contient les noms de toute les colonnes présente dans la table (de manière unique) avec table étant la table originel( pas la table qui est crée par le progamme mais celle qui est présent en mémoire) et la colonne associé à celle-ci
    TableNamesSet* Name;

public:
    Table(std::shared_ptr<std::vector<std::shared_ptr<Colonne>>> data_, TableNamesSet* Name_)
        : Name(Name_)
    {
        for (auto e : *data_) {
            for (auto n : e->get_name()->GetAllFullNames()) {
                m_Map[n] = e;
            }
            m_Map[e->get_name()->GetMainName()] = e;
            m_ColonnesNames.push_back(e->get_name());
        }
    }

    void Selection(const Parsing::BinaryExpression::Condition pred, const std::shared_ptr<std::unordered_set<ColonneNamesSet*>> nom_colonnes);
    void Projection(std::unique_ptr<std::unordered_set<ColonneNamesSet*>> ColumnToSave);

    int size()
    {
        return m_Map.size();
    }
    int Columnsize()
    {
        return m_Map[m_ColonnesNames[0]->GetMainName()]->size();
    }

    ColumnData get_value(ColonneNamesSet* column_name, int pos_ind)
    {
        auto temp = m_Map[column_name->GetMainName()];
        return temp->getValue(pos_ind);
    }

    bool colonne_exist(ColonneNamesSet clef_testé)
    {
        return !(m_Map.end() == m_Map.find(clef_testé.GetMainName())); // this test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
    }
    TableNamesSet* Get_name() { return Name; };

    std::unordered_map<std::string, std::shared_ptr<Colonne>> GetMap()
    {
        return m_Map;
    };

    std::vector<std::shared_ptr<Colonne>>* get_data_ptr()
    {
        auto res = new std::vector<std::shared_ptr<Colonne>>();

        res->reserve(m_Map.size());
        for (auto e : m_Map) {
            res->emplace_back(e.second);
        }

        return res;
    }
    std::vector<ColonneNamesSet*>* GetColumnNames() { return &m_ColonnesNames; }

    void Sort(ColonneNamesSet* ColonneToSortBy);

    std::vector<ColumnData>* GetSample(ColonneNamesSet* ColonneGetValueFrom)
    {
        std::vector<ColumnData>* ValSet = new std::vector<ColumnData>();
        ValSet->reserve(1000);
        for (int i = 0; i < 1000 and i < this->Columnsize(); i++) {
            int pos = int(std::max(i * (this->Columnsize() / 1000), i));
            auto temp = this->get_value(ColonneGetValueFrom, pos);
            ValSet->push_back(temp);
        }
        return ValSet;
    }
};

} // Database::QueryPlanning
//
#endif // !TABLE_OP
