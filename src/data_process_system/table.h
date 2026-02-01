#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include "algebrizer_types.h"

#include "namingsystem.h"
#include "racine.h"

#ifndef TABLE_OP_H

#define TABLE_OP_H

namespace Database::QueryPlanning {

class Table {
private:
    const TableNamesSet& m_Name;

    // permet de trouver la position d'une colonne à partir de son nom
    std::unique_ptr<std::unordered_map<std::string, int>> m_Map;

    // contient les noms de toute les colonnes présente dans la table (de manière unique) avec table étant la table originel
    // ( pas la table qui est crée par le progamme mais celle qui est présent en mémoire) et la colonne associé à celle-ci
    std::unique_ptr<std::vector<Racine>> m_Columns;

    std::unique_ptr<std::vector<int>> m_Indices; // indices valides dans racine

public:
    Table(std::vector<Racine>& data, const TableNamesSet& name)
        : m_Name(std::move(name))
    {
        m_Map = std::make_unique<std::unordered_map<std::string, int>>();

        m_Columns = std::make_unique<std::vector<Racine>>();

        m_Map->reserve(data.size());

        m_Columns->reserve(data.size());

        for (int i = 0; i < data.size(); i++) {
            auto e = std::move(data[i]);

            for (auto n : e.GetName().GetAllFullNames()) {
                m_Map->at(n) = i;
            }

            m_Map->at(e.GetName().GetMainName()) = i;

            m_Columns->push_back(std::move(e));
        }

        auto temp = std::make_unique<std::vector<int>>();

        temp->reserve(data[0].size());

        for (int i = 0; i < data[0].size(); i++) {
            temp->emplace_back(i);
        }

        m_Indices = std::move(temp);
    }

    Table(const Table& other)
        : m_Name(other.m_Name)
    {
    }

    const Table&
    operator=(const Table& other)
    {
        return other;
    }

    int size() const
    {
        return m_Map->size();
    }

    int Columnsize() const
    {
        return m_Indices->size();
    }

    [[nodiscard]] inline Racine& GetRacineFromMap(int i) const
    {
        return m_Columns->at(i);
    }

    [[nodiscard]] inline ColumnData GetValueFromTable(const ColonneNamesSet& column_name, int pos_ind) const
    {
        int rac_pos = m_Map->at(column_name.GetMainName());

        int real_i = m_Indices->at(pos_ind);

        Racine& rac = GetRacineFromMap(rac_pos);

        return rac.GetValueAt(real_i);
    }

    inline bool DoColumnExists(const ColonneNamesSet& tested_key) const
    {
        // this test if a colonne is already registered in a table, return true if the colonne exists and false if it doesn't
        return !(m_Map->end() == m_Map->find(tested_key.GetMainName()));
    }

    [[nodiscard]] inline Racine& GetRacinePtr(const ColonneNamesSet& column_name) const
    {
        int rac_pos = m_Map->at(column_name.GetMainName());

        return GetRacineFromMap(rac_pos);
    }

    std::vector<Racine>* GetColumns() const { return m_Columns.get(); }

    const TableNamesSet& GetName() const { return m_Name; }

    [[nodiscard]] std::unique_ptr<std::vector<int>> Sort(const ColonneNamesSet& column_to_sort) const
    {
        auto& col = GetRacinePtr(column_to_sort);

        auto pos_to_sort = std::make_unique<std::vector<int>>();

        pos_to_sort->resize(m_Indices->size());

        std::iota(pos_to_sort->begin(), pos_to_sort->end(), 0);

        std::sort(pos_to_sort->begin(), pos_to_sort->end(),
            [&](int a, int b) { return this->GetValueFromTable(column_to_sort, a) < this->GetValueFromTable(column_to_sort, b); });

        return pos_to_sort;
    }

    void ApplyFilter(const std::vector<int>& new_ind)
    {
        auto new_indices = std::make_unique<std::vector<int>>();

        new_indices->reserve(new_ind.size());

        for (auto e : new_ind) {
            new_indices->emplace_back(m_Indices->at(e));
        }

        m_Indices = std::move(new_indices);
    }

    void DeleteCol(const ColonneNamesSet& col_to_delete) const
    {
        for (int i = 0; i < m_Columns->size(); i++) {

            if (m_Columns->at(i).GetName() == col_to_delete) {

                for (auto s : col_to_delete.GetAllFullNames()) {
                    m_Map->erase(s);
                }

                m_Columns->erase(m_Columns->begin() + i);

                break;
            }
        }

        Update();
    }

    void Update() const
    {
        m_Map->erase(m_Map->begin(), m_Map->end());

        for (int i = 0; i < m_Columns->size(); i++) {

            auto& r = m_Columns->at(i);

            for (auto n : r.GetName().GetAllFullNames()) {
                m_Map->at(n) = i;
            }
        }
    }
};

} // Database::QueryPlanning
//
#endif // !TABLE_OP
