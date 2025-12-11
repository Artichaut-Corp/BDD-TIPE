#include <cmath>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../algebrizer_types.h"
#include "namingsystem.h"
#include "table.h"

#ifndef METATABLE_OP

#define METATABLE_OP

namespace Database::QueryPlanning {

class MetaTable {
private:
    std::unordered_map<std::string, std::shared_ptr<Table>> m_MapTableNameToTable; // permet de trouver la Table à partir de son nom
    std::vector<std::shared_ptr<Table>> m_Tables; // contient les noms de toute les tables présente dans la meta-table (de manière unique)
    std::unordered_map<std::string, std::shared_ptr<Table>> m_MapColNameToTable;

public:
    MetaTable(std::shared_ptr<Table> data_)
    {

        for (auto n : data_->get_name()->GetAllNames()) {
            m_MapTableNameToTable[n] = data_;
        }
        for (auto r : *data_->GetColumns()) {
            for (auto r : r->get_name()->GetAllFullNames()) {
                m_MapColNameToTable[r] = data_;
            }
        }
        m_Tables.push_back(data_);
    }

    void Selection(const Parsing::BinaryExpression::Condition pred, const std::shared_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> nom_colonnes);
    void Projection(std::unique_ptr<std::unordered_set<std::shared_ptr<ColonneNamesSet>>> ColumnToSave);

    int size()
    {
        return m_MapTableNameToTable.size();
    }
    int Columnsize()
    {
        return m_Tables[0]->Columnsize();
    }

    ColumnData get_value(std::shared_ptr<ColonneNamesSet> column_name, int pos_ind)
    {
        return m_MapColNameToTable[column_name->GetMainName()]->get_value_dans_table(column_name, pos_ind);
    }

    bool colonne_exist(std::shared_ptr<ColonneNamesSet> clef_testé)
    {
        return m_MapColNameToTable.contains(clef_testé->GetMainName());
    }

    

    std::shared_ptr<Table> GetTableByColName(std::shared_ptr<ColonneNamesSet> colname) { return m_MapColNameToTable[colname->GetMainName()]; }

    std::shared_ptr<Table> GetTableByTableName(TableNamesSet* tablename) { return m_MapTableNameToTable[tablename->GetMainName()]; }

    std::vector<std::shared_ptr<Table>>* GetTableNames() { return &m_Tables; }

    void Sort(std::shared_ptr<ColonneNamesSet> ColonneToSortBy);

    std::vector<ColumnData>* GetSample(std::shared_ptr<ColonneNamesSet> ColonneGetValueFrom)
    {
        auto table = m_MapTableNameToTable[ColonneGetValueFrom->GetTableSet()->GetMainName()];
        std::vector<ColumnData>* ValSet = new std::vector<ColumnData>();
        ValSet->reserve(1000);
        for (int i = 0; i < 1000 and i < table->Columnsize(); i++) {
            int pos = int(std::max(i * (table->Columnsize() / 1000), i));
            auto temp = table->get_value_dans_table(ColonneGetValueFrom, pos);
            ValSet->push_back(temp);
        }
        return ValSet;
    }

    void UpdateMetaTable()
    {
        m_MapTableNameToTable.erase(m_MapTableNameToTable.begin(), m_MapTableNameToTable.end());
        m_MapColNameToTable.erase(m_MapColNameToTable.begin(), m_MapColNameToTable.end());

        for (int i =0;i<m_Tables.size();i++) {
            m_Tables[i]->update();
            if (m_Tables[i]->size() == 0) {
                m_Tables.erase(m_Tables.begin()+i);
                i--;// if we delete a Table, we change the size and move all the vector to the left by 1 there fore we need to compensate it
            } else {
                for (auto n : m_Tables[i]->get_name()->GetAllNames()) {
                    m_MapTableNameToTable[n] = m_Tables[i];
                }
                for (auto r : *m_Tables[i]->GetColumns()) {
                    for (auto s : r->get_name()->GetAllFullNames()) {
                        m_MapColNameToTable[s] = m_Tables[i];
                    }
                }
            }
        }
    }

    void AppliqueOrdre(std::vector<int>* Ordre)
    {
        for (auto e : m_Tables) {
            e->AppliqueFiltre(Ordre);
        }
    }

    void FusionMetaTable(std::shared_ptr<MetaTable> table2)
    {
        m_Tables.insert(m_Tables.end(), table2->m_Tables.begin(), table2->m_Tables.end());
        UpdateMetaTable();
    }
    std::string Get_name() { return m_Tables[0]->get_name()->GetMainName(); }
    std::vector<std::shared_ptr<ColonneNamesSet>>* GetColumnNames()
    {
        auto vec = new std::vector<std::shared_ptr<ColonneNamesSet>> {};
        vec->reserve(m_MapColNameToTable.size());
        for (auto t : m_Tables) {
            for (auto r : *t->GetColumns()) {
                vec->push_back(r->get_name());
            }
        }
        return vec;
    }
};

} // Database::QueryPlanning
//
#endif // !METATABLE_OP
