#include <memory>
#include <unordered_map>
#include <vector>

#include "algebrizer_types.h"

#include "namingsystem.h"
#include "table.h"

#ifndef METATABLE_OP

#define METATABLE_OP

namespace Database::QueryPlanning {

class MetaTable {
private:
    // contient les noms de toute les tables présente dans la meta-table (de manière unique)
    std::vector<std::unique_ptr<Table>> m_Tables;

    // permet de trouver la Table à partir de son nom
    std::unordered_map<std::string, int> m_MapTableNameToTable;

    std::unordered_map<std::string, int> m_MapColNameToTable;

public:
    MetaTable(std::unique_ptr<Table> table)
    {
        m_MapTableNameToTable = std::unordered_map<std::string, int>();

        m_MapColNameToTable = std::unordered_map<std::string, int>();

        m_Tables = std::vector<std::unique_ptr<Table>>();

        m_Tables.reserve(1);

        for (auto& name : table->GetName().GetAllNames()) {
            m_MapTableNameToTable.insert({ name, 0 });
        }

        for (auto* r : *table->GetColumns()) {
            for (auto r : r->GetName().GetAllFullNames()) {
                m_MapColNameToTable.insert({ r, 0 });
            }
        }

        m_Tables.emplace_back(std::move(table));
    }

    MetaTable(std::vector<Racine*>& data, const TableNamesSet& name)
    {

        m_MapTableNameToTable = std::unordered_map<std::string, int>();

        m_MapColNameToTable = std::unordered_map<std::string, int>();

        m_Tables = std::vector<std::unique_ptr<Table>>();

        m_Tables.emplace_back(std::make_unique<Table>(data, name));

        for (auto& name : m_Tables.at(0)->GetName().GetAllNames()) {
            m_MapTableNameToTable.insert({ name, 0 });
        }

        for (auto r : *m_Tables.at(0)->GetColumns()) {
            for (auto r : r->GetName().GetAllFullNames()) {
                m_MapColNameToTable.insert({ r, 0 });
            }
        }
    }

    void Selection(const Parsing::BinaryExpression::Condition& pred, const std::unique_ptr<std::unordered_set<ColonneNamesSet*>> name_columns);

    void Projection(std::unique_ptr<std::unordered_set<const ColonneNamesSet*>> columns_to_save);

    inline int Length() const
    {
        return m_MapTableNameToTable.size();
    }

    inline int Columnsize() const
    {
        return m_Tables.at(0)->Columnsize();
    }

    inline const Table& GetTableFromMap(int i) const
    {
        return *m_Tables.at(i).get();
    }

    const Table& GetTableByColName(const ColonneNamesSet& colname) const
    {
        int i = m_MapColNameToTable.at(colname.GetMainName());

        return GetTableFromMap(i);
    }

    const Table& GetTableByTableName(TableNamesSet* tablename) const
    {
        int i = m_MapTableNameToTable.at(tablename->GetMainName());

        return GetTableFromMap(i);
    }

    ColumnData GetValue(const ColonneNamesSet& column_name, int pos_ind) const
    {
        const Table& t = GetTableFromMap(m_MapColNameToTable.at(column_name.GetMainName()));

        return t.GetValueFromTable(column_name, pos_ind);
    }

    bool IsExisting(const ColonneNamesSet& tested_key) const
    {
        return m_MapColNameToTable.contains(tested_key.GetMainName());
    }

    std::vector<std::unique_ptr<Table>>& GetTableNames() { return m_Tables; }

    void Sort(const ColonneNamesSet& ColonneToSortBy);

    std::unique_ptr<std::vector<ColumnData>> GetSampleFromColumn(const ColonneNamesSet& column_name)
    {
        const Table& table = GetTableByColName(column_name);

        auto values_set = std::make_unique<std::vector<ColumnData>>();

        values_set->reserve(1000);

        for (int i = 0; i < 1000 and i < table.Columnsize(); i++) {

            int pos = int(std::max(i * (table.Columnsize() / 1000), i));

            auto temp = table.GetValueFromTable(column_name, pos);

            values_set->emplace_back(temp);
        }
        return values_set;
    }

    void UpdateMetaTable()
    {
        m_MapTableNameToTable.erase(m_MapTableNameToTable.begin(), m_MapTableNameToTable.end());

        m_MapColNameToTable.erase(m_MapColNameToTable.begin(), m_MapColNameToTable.end());

        for (int i = 0; i < m_Tables.size(); i++) {
            m_Tables.at(i)->Update();

            if (m_Tables.at(i)->size() == 0) {
                // if we delete a Table, we change the size and move all the vector to the left by 1 there fore we need to compensate it
                m_Tables.erase(m_Tables.begin() + i);
                i--;
            } else {

                for (auto n : m_Tables.at(i)->GetName().GetAllNames()) {
                    m_MapTableNameToTable.insert({ n, i });
                }

                for (auto r : *m_Tables.at(i)->GetColumns()) {
                    for (auto s : r->GetName().GetAllFullNames()) {
                        m_MapColNameToTable.insert({ s, i });
                    }
                }
            }
        }
    }

    void AppliqueOrdre(const std::vector<int>& order)
    {   
        for (auto& e : m_Tables) {
            e->ApplyFilter(order);
        }
    }

    void FusionMetaTable(MetaTable& other)
    {
        int nbr_other_tables = other.m_Tables.size();

        m_Tables.reserve(m_Tables.size() + nbr_other_tables);
        m_Tables.insert(
            m_Tables.end(),
            std::make_move_iterator(other.m_Tables.begin()),
            std::make_move_iterator(other.m_Tables.end()));

        UpdateMetaTable();
    }

    std::string GetName() { return m_Tables.at(0)->GetName().GetMainName(); }

    std::unique_ptr<std::vector<std::reference_wrapper<ColonneNamesSet>>> GetColumnNames()
    {
        auto vec = std::make_unique<std::vector<std::reference_wrapper<ColonneNamesSet>>>();

        vec->reserve(m_MapColNameToTable.size());

        for (auto& t : m_Tables) {
            for (auto r : *t->GetColumns()) {
                vec->emplace_back(r->GetName());
            }
        }

        return vec;
    }
};

} // Database::QueryPlanning
//
#endif // !METATABLE_OP
